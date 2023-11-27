import example_module
def on_ping(message):
    example_module.pong(f"python has read your message: {message}")
    example_module.pong(f"from python: thank you for your kind words!")