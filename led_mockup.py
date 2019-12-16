#!/usr/bin/python -tt

'''
Simulate an LED strip in a terminal to mockup patterns quickly.

This requires supporting full RGB escape sequences to work correctly.
<ESC> [ 38 ; 2 ; FGR ; FGG ; FGB m

By default this also uses UTF-8 circle characters to represent the LEDs.

Terminals:
  * Mac OS Terminal may not work
  * iTerm 2 works
  * Windows Terminal (Preview) works
  * Windows Bash Shell lacks the circle character

Issues:
  * LEDs must fit on a single line
  * Outputs color codes for every LED even if color isn't changing
  * This isn't polished; it's a quick a dirty hack to speed up itertion
    when designing modes
'''

import collections
import colorsys
import math
import random
import sys
import time

try:
  import fcntl
  import os
  import struct
  import termios

  def get_terminal_size():
    # pylint: disable=invalid-name
    '''Get terminal size returning width x height'''
    env = os.environ
    def ioctl_gwinsz(fd):
      '''Get size via ioctl'''
      try:
        cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ, '1234'))
      except Exception:
        return
      else:
        return cr
    cr = ioctl_gwinsz(0) or ioctl_gwinsz(1) or ioctl_gwinsz(2)
    if not cr:
      try:
        fd = os.open(os.ctermid(), os.O_RDONLY)
        cr = ioctl_gwinsz(fd)
        os.close(fd)
      except Exception:
        pass
    if not cr:
      cr = (env.get('LINES', 25), env.get('COLUMNS', 80))

    return int(cr[1]), int(cr[0])
except Exception:
  def get_terminal_size():
    '''Dummy version that always returns 80x24'''
    return (80, 24)


class LED(object):
  '''Represent a single LED'''
  def __init__(self, red, green, blue):
    self._red = red
    self._green = green
    self._blue = blue

  @property
  def red(self):
    '''Get red'''
    return self._red

  @red.setter
  def red(self, value):
    '''Set red'''
    self._red = value % 256

  @property
  def green(self):
    '''Get green'''
    return self._green

  @green.setter
  def green(self, value):
    '''Set green'''
    self._green = value % 256

  @property
  def blue(self):
    '''Get blue'''
    return self._blue

  @blue.setter
  def blue(self, value):
    '''Set blue'''
    self._blue = value % 256

  def colorcode(self):
    '''Return ANSI colorcode'''
    # 38;2;FGR;FGG;FGBm sets colors via RGB values (0-255)
    return '\x1B[38;2;%d;%d;%dm' % (self._red, self._green, self._blue)

  def hsv(self, hue, saturation, value):
    '''Set color using HSV values'''
    raw_red, raw_green, raw_blue = colorsys.hsv_to_rgb(
        hue / 360.0,
        saturation / 100.0,
        value / 100.0)
    self.red = int(raw_red * 255)
    self.green = int(raw_green * 255)
    self.blue = int(raw_blue * 255)


class LEDString(object):
  '''Represent an LED String'''
  def __init__(self, size, symbol=u'\u25cf'):
    self.size = size
    self.leds = [LED(255, 255, 255) for _ in xrange(size)]
    if isinstance(symbol, unicode):
      self.symbol = symbol.encode('utf-8')
    else:
      self.symbol = symbol  # pylint: disable=redefined-variable-type

    width, _height = get_terminal_size()
    self.prefix = ' ' * max((width - self.size) / 2, 0)

    self.header()

  def __len__(self):
    return self.size

  def __setitem__(self, led, value):
    if led < 0 or led >= self.size:
      raise Exception('led[%d] outside of range [0...%d]' % (led, self.size))

    if not isinstance(value, LED):
      raise TypeError('value is not of type LED')

    self.leds[led] = value

  def __getitem__(self, led):
    if led < 0 or led >= self.size:
      raise Exception('led[%d] outside of range [0...%d]' % (led, self.size))
    return self.leds[led]

  def __iter__(self):
    for led in self.leds:
      yield led

  def __reversed__(self):
    for led in reversed(self.leds):
      yield led

  def header(self):
    '''Draw header'''
    # Clear screen
    #   2J: Clear screen
    #   3J: Clear screen and scrollback buffer
    #   H: Home cursor
    #   ?25l: Hide cursor
    sys.stdout.write('\x1B[3J\x1B[H\x1B[2J\x1B[?25l')

    sys.stdout.write(self.prefix)
    sys.stdout.write('Using %d LEDs\n\n\n' % self.size)

    remaining = (self.size - 1) % 10

    sys.stdout.write(self.prefix)
    for i in xrange(0, self.size, 10):
      if i == 0:
        sys.stdout.write('0')
      else:
        sys.stdout.write('%10d' % i)
    if remaining >= 4:
      sys.stdout.write('%*d' % (remaining, self.size-1))
    sys.stdout.write('\n')


    sys.stdout.write(self.prefix)
    for i in xrange(0, self.size, 10):
      if i > 0:
        sys.stdout.write(' ' * 9)
      sys.stdout.write('|')
    if remaining >= 4:
      sys.stdout.write('|'.rjust(remaining))
    sys.stdout.write('\n')

  def draw(self):
    '''Draw LED strip'''
    sys.stdout.write('\x1B[6;1H')
    sys.stdout.write(self.prefix)
    sys.stdout.write(
        ''.join('%s%s' % (led.colorcode(), self.symbol) for led in self.leds))
    sys.stdout.write('\x1B[0K\r')


class Mode(object):
  '''LED Mode Abstract Base'''
  def __init__(self, leds):
    self.leds = leds

  def loop(self):
    '''Process one single loop'''
    raise NotImplementedError


class Rainbow(Mode):
  '''Draw rainbow gradient'''
  def __init__(self, *args, **kwargs):
    super(Rainbow, self).__init__(*args, **kwargs)

    self.hue = 0
    self.step = 359.0 / len(self.leds)

  def loop(self):
    '''Process one single loop'''
    for index in xrange(len(self.leds)):
      self.leds[index].hsv((self.hue + int(self.step * index)) % 360, 100, 100)
    self.hue = (self.hue + 2) % 360


class ColorCycle(Mode):
  '''Draw solid color which changes over time'''
  def __init__(self, *args, **kwargs):
    super(ColorCycle, self).__init__(*args, **kwargs)

    self.hue = 0

  def loop(self):
    '''Process one single loop'''

    for led in self.leds:
      led.hsv(self.hue, 100, 100)
    self.hue = (self.hue + 1) % 360


class Scanner(Mode):
  '''Draw a Larson Scanner (Knight Rider, Battlestar Galactica)'''
  def __init__(self, *args, **kwargs):
    super(Scanner, self).__init__(*args, **kwargs)

    self.positions = collections.deque((0,), maxlen=5)
    self.fade = 0.90
    self.step = 1
    self.calls = 0

  def loop(self):
    '''Process one single loop'''
    self.calls = self.calls + 1
    if self.calls % 3 != 0:
      return

    for led in self.leds:
      led.red = int(led.red * self.fade)
      led.green = int(led.green * self.fade)
      led.blue = int(led.blue * self.fade)

    for index in self.positions:
      led = self.leds[index]
      led.red = 255
      led.green = 0
      led.blue = 0

    new_position = self.positions[0] + self.step
    if new_position >= len(self.leds) or new_position < 0:
      self.step = -self.step
      new_position = self.positions[0] + self.step
    self.positions.appendleft(new_position)


class Twinkle(Mode):
  '''Twinkle an LED'''
  class State(object):
    def __init__(self, step):
      self.step = step
      self.waiting = True

  def __init__(self, *args, **kwargs):
    '''
    Configuration values:
      hue: LED Hue
      min_flicker_delay: Minimum iteration between flickers
      max_flicker_delay: Maximum iteration between flickers
      flicker_length: Amount of iterations a flicker lasts
    '''
    super(Twinkle, self).__init__(*args, **kwargs)

    self.hue = kwargs.get('hue', random.randint(0, 359))
    self.min_flicker_delay = kwargs.get('min_flicker_delay', 50)
    self.max_flicker_delay = kwargs.get('max_flicker_delay', 500)
    flicker_length = kwargs.get('flicker_length', 150)

    for led in self.leds:
      led.hsv(0, 0, 0)
      self.states = [Twinkle.State(self.flicker_delay()) for _ in self.leds]

    self.levels = [
        int(abs((50 * math.cos(4 * math.pi / flicker_length * i)) - 50) + 0.5)
        for i in xrange(flicker_length)
    ]

  def flicker_delay(self):
    '''Return a number of iterations to delay until the next flicker'''
    return random.randint(self.min_flicker_delay, self.max_flicker_delay)

  def loop(self):
    '''Process one single loop'''
    for index in xrange(len(self.leds)):
      state = self.states[index]
      if state.step <= 0:
        if state.waiting:
          state.step = len(self.levels) - 1
        else:
          state.step = self.flicker_delay()
        state.waiting = not state.waiting

      if not state.waiting:
        self.leds[index].hsv(self.hue, 100, self.levels[state.step])
      state.step -= 1


class SaturationFader(Mode):
  def __init__(self, *args, **kwargs):
    '''
    Configuration values:
      hue: LED Hue
    '''
    super(SaturationFader, self).__init__(*args, **kwargs)

    self.offset = 0
    self.hue = kwargs.get('hue', random.randint(0, 359))
    self.step = float(len(self.leds)) / 100

  def loop(self):
    '''Process one single loop'''
    for index in xrange(len(self.leds)):
      saturation = int((math.sin(2 * math.pi / 100 * (index + self.offset)) * 50) + 50.5)
      self.leds[index].hsv(self.hue, saturation, 100)
    self.offset = (self.offset + 1) % len(self.leds)


def main():
  '''Main routine'''
  leds = LEDString(100)

  #mode = Twinkle(leds)
  mode = SaturationFader(leds)

  while True:
    mode.loop()
    leds.draw()
    time.sleep(0.01)


if __name__ == '__main__':
  try:
    main()
  except KeyboardInterrupt:
    pass
  finally:
    # m: Reset color to default
    # ?25h: Show cursor
    sys.stdout.write('\x1B[m\x1B[?25h\n')
