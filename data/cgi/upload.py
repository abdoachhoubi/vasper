from flask import Flask, render_template, request

app = Flask(__name__)

# Define a route to display the upload form
@app.route('/', methods=['GET', 'POST'])
def upload_file():
    if request.method == 'POST':
        uploaded_file = request.files['file']
        if uploaded_file:
            # Save the uploaded file to a directory
            uploaded_file.save('uploads/' + uploaded_file.filename)
            return 'File uploaded successfully.'

    return render_template('upload.html')

if __name__ == '__main__':
    app.run(debug=True)
