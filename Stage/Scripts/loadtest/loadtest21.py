import example_module
def on_load_1():
    example_module.event_handled("event handled: on_load_1")
    
def on_load_3():
    example_module.event_handled("event handled: on_load_3")
    
def on_load_5():
    example_module.event_handled("event handled: on_load_4")