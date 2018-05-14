from flask import Flask, request
from flask_restful import Resource, Api
from numpy import random

app = Flask(__name__)
api = Api(app)

todos = {}

class HelloWorld(Resource):
    def get(self):
        params = request.args
        print(params)
        return {'status': str(random.choice([1,0]))}

api.add_resource(HelloWorld, '/')

if __name__ == '__main__':
    app.run(host='0.0.0.0',debug=True)
    #app.run(debug=True)




