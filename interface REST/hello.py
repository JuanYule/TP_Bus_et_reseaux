from flask import Flask
from flask import jsonify, render_template, abort, request
import json
from fonctions import *

app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello, World!\n'

welcome = "Welcome to 3ESE API!"
temperature = []

@app.route('/api/welcome/', methods=['GET', 'POST', 'DELETE'])
def api_welcome():
    global welcome
    if request.method == 'POST':
        welcome = request.get_json()['data']
        return "", 202
    if request.method == 'DELETE':
        welcome = ''
        return "", 205
    return jsonify({"text":welcome})
    
@app.route('/api/welcome/<int:index>', methods=['GET','PUT', 'PATCH', 'DELETE'])
def api_welcome_index(index):
    global welcome
    if(index >= len(welcome)):
        abort(404)
    else:
        if(request.method == 'GET'):
            return jsonify({"index": index, "val": welcome[index]}), {"Content-Type": "application/json"}
        elif(request.method == 'PATCH'):
            welcome = welcome[:index]+ request.get_json()['data'] + welcome[index+1:]
            return "", 202
        elif(request.method == 'PUT'):
            welcome = welcome[:index]+ request.get_json()['data'] +  welcome[index:]
            return "", 202
        elif(request.method == 'DELETE'):
            welcome = welcome[:index] + welcome[index+1:]
            return "", 205

@app.route('/api/request/', methods=['GET', 'POST'])
@app.route('/api/request/<path>', methods=['GET','POST'])
def api_request(path=None):
    resp = {
            "method":   request.method,
            "url" :  request.url,
            "path" : path,
            "args": request.args,
            "headers": dict(request.headers),
    }
    if request.method == 'POST':
        resp["POST"] = {
                "data" : request.get_json(),
                }
    return jsonify(resp)
    
    
@app.route('/temp/', methods=['GET', 'POST'])
def functio_temp():
    global temperature
    if request.method == 'GET':
        verification("GET_T")
        send_to_rasp = ''.join(data_temp)
        return jsonify({"text":data_test})

@app.route('/temp/<int:index>', methods=['GET', 'POST', ])

@app.route('/pres/', methods=['GET', 'DELETE', ])

@app.route('/pres/<int:index>', methods=['GET', 'DELETE', ])

@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404