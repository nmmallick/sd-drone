import RPi.GPIO as GPIO
import threading
import copy

class ToggleButton:
    def __init__(self, pin, cb=None):

        self.stop = False
        self.pin = pin
        self.value = False
        self.callback = cb
        
        GPIO.setwarnings(False)
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

        self.mutex = threading.Lock()
        self.thread = threading.Thread(target=self.loop, daemon=True)
        self.thread.start()

    def loop(self):
        while not self.stop:
            ret = GPIO.wait_for_edge(self.pin, GPIO.FALLING, timeout=500)
            if ret is None:
                continue
            
            if self.callback != None:
                self.callback()

            with self.mutex:
                self.value = not self.value

    def getValue(self):
        with self.mutex:
            ret = copy.deepcopy(self.value)
        return ret

    def __del__(self):
        self.stop = True
        self.thread.join()
        
if __name__ == '__main__':
    button = ToggleButton(17)
    prev_value = button.getValue()
    
    while True:
        if button.getValue() != prev_value:
            prev_value = button.getValue()
            print("State : " + str(prev_value))




