import os
import sys
import subprocess

# Auto-install dotenv if missing
try:
    from dotenv import load_dotenv
except ImportError:
    subprocess.check_call([sys.executable, "-m", "pip", "install", "python-dotenv"])
    from dotenv import load_dotenv

Import("env")

# --- LOAD DOTENV ---
env_path = os.path.join(env.get("PROJECT_DIR"), ".env")
if os.path.exists(env_path):
    load_dotenv(env_path)
    
    # Get values from .env
    ssid = os.getenv("WIFI_SSID")
    password = os.getenv("WIFI_PASS")
    server_ip = os.getenv("SERVER_IP")

    # This is the "Magic" for PlatformIO:
    # We need to apply these to BOTH 'env' and 'projenv'
    # 'projenv' is specifically for the files in your /src folder
    
    # Build the list of macros
    # We use double escaping for the quotes so they survive the shell
    my_flags = [
        ("WIFI_SSID", f'\\"{ssid}\\"'),
        ("WIFI_PASS", f'\\"{password}\\"'),
        ("SERVER_IP", f'\\"{server_ip}\\"')
    ]

    # Apply to global environment
    env.Append(CPPDEFINES=my_flags)

    # Apply specifically to the Project Environment (the src/ folder)
    # We use a try/except because projenv only exists during certain build phases
    try:
        Import("projenv")
        projenv.Append(CPPDEFINES=my_flags)
        print(f"--- Successfully injected {ssid} into projenv ---")
    except:
        print("--- projenv not ready, using default env ---")

else:
    print(f"--- ERROR: .env file NOT found at {env_path} ---")