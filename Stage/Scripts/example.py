import example_module
def on_ping(message):
    response = message.replace("c++", "python")
    example_module.pong(response)