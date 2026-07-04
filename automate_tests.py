import os
from dotenv import load_dotenv
from google import genai

# --- CONFIGURATION ---
ENGINE_CPP = "engine/OrderBook.cpp"

def ai_architectural_review():

    load_dotenv()


    api_key = os.environ.get("GEMINI_API_KEY")

    if not api_key:
        print("[AI] Skipping AI review: GEMINI_API_KEY not found in .env file.")
        return

    print("[*] Sending Tartarus Core to AI for edge-case analysis...")

    client = genai.Client(api_key=api_key)

    try:
        with open(ENGINE_CPP, "r") as f:
            cpp_code = f.read()

        prompt = f"""
        You are a Principal Software Engineer doing a code review on a highly optimized C++ Limit Order Book for a pure software engineering role.
        Focus strictly on software architecture, pointer safety, iterator invalidation, memory overhead, and time complexity. Do not use any hardware analogies.
        
        Here is the core matching logic:
        {cpp_code}
        
        Provide 3 highly specific, technical bullet points on how this code handles partial fills and memory safely. 
        Output plain text formatting only. Be extremely concise.
        """

        response = client.models.generate_content(
            model='gemini-2.5-flash',
            contents=prompt
        )

        print("\n=== AI ARCHITECTURAL REVIEW ===")
        print(response.text.strip())
        print("===============================\n")

    except FileNotFoundError:
        print(f"[ERROR] Could not find {ENGINE_CPP}. Ensure the path is correct.")
    except Exception as e:
        print(f"[ERROR] API Call Failed: {e}")

def run_cpp_main():
    """
    Build and run main.cpp using CMake from project root.
    Creates a 'build' directory, runs cmake, builds, and executes the generated binary if successful.
    """
    import subprocess
    import re

    build_dir = "build"
    os.makedirs(build_dir, exist_ok=True)
    try:
        # Configure
        subprocess.check_call(["cmake", ".."], cwd=build_dir)
        # Build
        subprocess.check_call(["cmake", "--build", "."], cwd=build_dir)
        # Parse CMakeLists.txt to find add_executable target name
        cmake_path = os.path.join(os.getcwd(), "CMakeLists.txt")
        target_name = None
        try:
            with open(cmake_path, "r") as cf:
                cmake_text = cf.read()
            m = re.search(r'add_executable\s*\(\s*([A-Za-z_][A-Za-z0-9_\-]*)', cmake_text)
            if m:
                target_name = m.group(1)
        except FileNotFoundError:
            pass

        possible_bins = []
        if target_name:
            possible_bins.append(target_name)
        # Also include common fallbacks
        possible_bins.extend(["tartarus", "main"])

        for b in possible_bins:
            bin_path = os.path.join(build_dir, b)
            if os.path.isfile(bin_path) and os.access(bin_path, os.X_OK):
                subprocess.check_call([bin_path])
                return
        print("[WARN] Built, but no runnable binary found in build/. Check CMakeLists.txt for target name.")
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Building or running C++ project failed: {e}")


if __name__ == "__main__":
    print("========================================")
    print("      TARTARUS: AI QA PIPELINE          ")
    print("========================================\n")
    ai_architectural_review()
    # Also attempt to build and run C++ main via CMake
    run_cpp_main()