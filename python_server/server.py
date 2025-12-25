# Server.py (Using Flask)
from flask import Flask, render_template, request
import os

app = Flask(__name__)

texte_a_afficher = "Bonjour le monde !"  # Default text to display
textColor = (255, 255, 255)  # White color
newTexte = True
newColor = True

# Load all pixel files into a dictionary on startup
def load_pixel(char):
    with open(os.path.join("charPixels", char + ".txt"), 'r') as f:
                # Read the actual data lines, skipping metadata lines starting with '#'
                f.readline()
                size = f.readline().split(" ")[2]
                data = [line.strip() for line in f if not line.startswith('#')]
                # Join all rows into a single string of 0s and 1s
                PIXEL_DATA = "".join(data)

                return size + PIXEL_DATA


@app.route('/char/<char_request>', methods=['GET'])
def get_char_pixels(char_request):
    char = char_request
    try :
        char = load_pixel(char)
    except:
        char = False
    if char :
        # Returns the string of 0s and 1s (the full 900-pixel tab)
        return char, 200 
    else:
        return "Character not found", 404
    
@app.route('/firstText', methods=['GET'])
def get_first_texte():
    global texte_a_afficher, textColor
    return texte_a_afficher, 200

@app.route('/firstColor', methods=['GET'])
def get_first_color():
    global textColor
    color_string = ",".join(map(str, textColor))
    return color_string, 200

@app.route('/color', methods=['GET'])
def get_color():
    global newColor
    global textColor
    if newColor :
        newColor = False

        color_string = ",".join(map(str, textColor))
        return color_string, 200
    else :
        return "No new color", 204
    
@app.route('/text', methods=['GET'])
def get_texte():
    global newTexte
    global texte_a_afficher
    if newTexte :
        newTexte = False
        return texte_a_afficher, 200
    else :
        return "No new text", 204
    
def hexToRgb(hex_color):
    print(f"Converting hex color: {hex_color}")
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))

@app.route('/', methods=['GET', 'POST'])
def index():
    global newTexte
    global texte_a_afficher
    global textColor
    global newColor
    if request.method == 'POST':

        if 'message' in request.form and request.form['message'] != "": 
            message = request.form['message']
            print(f"Received message to display: {message}")
            texte_a_afficher = message
            newTexte = True
        
        color = request.form.get('color')
        if color is not None and color != "":
            rgb_color = hexToRgb(color)
            print(f"Received color to display: {color}")
            if rgb_color != textColor :
                textColor = rgb_color
                print(f"Text color changed to: {textColor}")
                newColor = True


        # Here you can add code to handle the message, e.g., send it to the LED display
    initial_hex = '#%02x%02x%02x' % textColor
    return render_template('index.html', current_color=initial_hex)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000) # Run on your local network IP