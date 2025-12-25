from PIL import Image, ImageDraw, ImageFont
import string
import os

# --- Configuration ---
FONT_HEIGHT = 28
FONT_FILE = "Terminus.ttf"  # <--- CHANGE THIS IF YOUR FILENAME IS DIFFERENT!
padding = 1
OUTPUT_DIR = "charPixels"
is_default_font = False

def create_output_directory(dir_name):
    """Creates the output directory if it doesn't exist."""
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)
        print(f"✅ Created directory: '{dir_name}'")
    else:
        print(f"💡 Directory '{dir_name}' already exists.")

# The full set of characters you requested
ALL_CHARS = string.ascii_letters + string.digits
# Example: 'aAbBcCdD...xXyYzZ0123456789'

# Dictionary to store the pixel data for every character
pixel_data_tabs = {}

# --- Font Loading ---
try:
    # Load the Terminus TrueType Font
    font = ImageFont.truetype(FONT_FILE, FONT_HEIGHT)
    print(f"✅ Successfully loaded font: {FONT_FILE} at size {FONT_HEIGHT}\n")
except IOError:
    print(f"❌ Error: Could not find or load '{FONT_FILE}'.")
    print("Please make sure the font file is in the same directory as the script.")
    # Exiting because the primary goal is to use Terminus
    exit()

# --- Main Processing Loop ---
print("--- Generating Pixel Data ---")

create_output_directory(OUTPUT_DIR)

for target_character in ALL_CHARS:
    try:
        # 1. Get accurate bounding box and calculate canvas size
        bbox = font.getbbox(target_character)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]

        canvas_width = text_width + 2 * padding
        canvas_height = text_height + 2 * padding

        # 2. Image Creation and Rendering
        # Grayscale image: black=0, white=255
        img = Image.new('L', (canvas_width, canvas_height), color='white')
        draw = ImageDraw.Draw(img)
        draw.text((padding, padding), target_character, fill=0, font=font)

        # 3. Pixel Data Extraction and Formatting (The Change is Here)
        pixel_data = list(img.getdata())
        
        file_content = []
        for i in range(0, len(pixel_data), canvas_width):
            row_pixels = pixel_data[i:i + canvas_width]
            
            # Map grayscale values (0=Black, 255=White) to Binary (1=Black, 0=White)
            binary_row = []
            for value in row_pixels:
                # If the value is close to black (0), map it to 1
                # Since Terminus is a bitmap font, this threshold is generous
                if value < 100: 
                    binary_row.append('1') # Foreground/ON
                else:
                    binary_row.append('0') # Background/OFF
                    
            # Join the 0s and 1s into a string separated by spaces
            row_values = "".join(binary_row)
            file_content.append(row_values)
        
        # 4. Save to File
        file_name = os.path.join(OUTPUT_DIR, f"{target_character}.txt")
        with open(file_name, 'w') as f:
            # Add updated metadata
            f.write(f"# Character: {target_character}\n")
            f.write(f"# Dimensions: {canvas_width}x{canvas_height}\n")
            f.write("# Data: Binary values (1=Foreground/ON, 0=Background/OFF)\n")
            
            # Write the pixel grid data
            f.write('\n'.join(file_content))

        print(f"Saved: {file_name}")
            
    except Exception as e:
        print(f"Warning: Failed to process character '{target_character}'. Error: {e}")

print("\n--- Processing Complete ---")
print(f"Binary pixel data is now in the '{OUTPUT_DIR}' folder.")