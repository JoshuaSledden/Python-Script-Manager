import example_module
def on_message(user, message):
    if user is not None:
        example_module.send_message(f"Python recieved a message from user: {user.id}. message: {message}")
    else:
        example_module.send_message(f"Python recieved a message from an unknown user. message: {message}")