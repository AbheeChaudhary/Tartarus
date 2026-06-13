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

if __name__ == "__main__":
    print("========================================")
    print("      TARTARUS: AI QA PIPELINE          ")
    print("========================================\n")
    ai_architectural_review()