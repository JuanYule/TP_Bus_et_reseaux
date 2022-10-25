from flask import Flask
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello, World!\n'

welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/')
def api_welcome():
    return welcome
    
@app.route('/api/welcome/<int:index>')
def api_welcome_index(index):
    return welcome[index]
