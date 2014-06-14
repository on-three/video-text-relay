# vim: set ts=2 expandtab:
"""

Module: irc.py
Desc: Helpers for IRC clients (msg decoding, paring, formatting)
Author: on_three
Email: on.three.email@gmail.com
DATE: Tuesday, Dec 24th 2013
  
""" 

import re
import string

def split_speaker(user):
  '''
  split a nick most likely in the form:
  "SoapKills!~ring@banana.phone"
  or (nick)!~(realname)@(vhost)
  into a tuble of (nick, vhost)
  '''
  nick, vhost = string.split(user, '!', maxsplit=1)
  if not vhost:
    vhost = 'nick@unknown'
  vhost = vhost.replace('~', '', 1)
  return nick, vhost

#after http://stackoverflow.com/questions/938870/python-irc-bot-and-encoding-issue
def decode(bytes):
  try:
    text = bytes.decode('utf-8')
  except UnicodeDecodeError:
    try:
      text = bytes.decode('iso-8859-1')
    except UnicodeDecodeError:
      text = bytes.decode('cp1252')
  return text


def encode(bytes):
  try:
    text = bytes.encode('utf-8')
  except UnicodeEncodeError:
    try:
      text = bytes.encode('iso-8859-1')
    except UnicodeEncodeError:
      text = bytes.encode('cp1252')
  return text

color_dict = {
  0 : 'white',
  1 : 'black',
  2 : 'dark blue',
  3 : 'dark green',
  4 : 'red',
  5 : 'dark red',
  6 : 'dark violet',
  7 : 'dark orange',
  8 : 'yellow',
  9 : 'light green',
  10 : 'cyan',
  11 : 'light cyan',
  12 : 'blue',
  13 : 'violet',
  14 : 'dark gray',
  15 : 'light gray',
  }

def color_code_to_X11(code):
  if not isinstance(code, int):
    return 'black'
  if code<0:
    return 'black'
  if code>15:
    return 'black'
  return color_dict[code]

def formatting_to_pango_markup(msg):
  '''
  Take an irc message already decoded from utf-8/latin1 etc
  and replace typical binary irc color code formatting with
  pango markup.
  '''
  msg = decode(msg)
  msg = re.sub(r'&','&amp;', msg)
  msg = re.sub(r'\<','&lt;', msg)
  msg = re.sub(r'\>','&gt;', msg)
  class MarkupFunctor(object):
    def __init__(self):
      self.match_found = False
      self.bold = False
      self.underline = False
      self.fg_color = -1
      self.bg_color = -1
      self.italic = False
      
    def __call__(self, match):
      '''
      function call operator called by regex replace
      upon the finding of a match in msg sring
      '''
      if match.groupdict()['reset'] is not None:
        self.bold = False
        self.underline = False
        self.fg_color = -1
        self.bg_color = -1
        self.italic = False

      if match.groupdict()['reverse'] is not None:
        c = self.bg_color
        self.bg_color = self.fg_color
        self.fg_color = c

      if match.groupdict()['bold'] is not None:
        self.bold = not self.bold

      if match.groupdict()['underline'] is not None:
        self.underline = not self.underline

      if match.groupdict()['italic'] is not None:
        self.italic = not self.italic

      if match.groupdict()['color'] is not None:
        if match.groupdict()['fg'] is not None:
          self.fg_color = int(match.groupdict()['fg'])
        if match.groupdict()['bg'] is not None:
          self.bg_color = int(match.groupdict()['bg'])
        if match.groupdict()['fg'] is None and match.groupdict()['bg'] is None:
          self.fg_color = -1
          self.bg_color = -1

      output = ''

      if not self.match_found:
        self.match_found=True
        output = '<span '
      else:
        output = '</span><span '

      if self.bold:
        output += 'weight="bold" '

      if self.underline:
        output += 'underline="single" '

      if self.italic:
        output += 'style="italic" '

      if self.fg_color >=0:
        output += 'color="' + color_code_to_X11(self.fg_color) + '" '

      if self.bg_color >=0:
        output += 'background="' +color_code_to_X11(self.bg_color) + '" '

      output += '>'

      return output

  #TODO: it would be best to refactor this to match not only the IRC binary markup
  #but also the text between this markup and the next instance/eol. That
  #would be easier to turn into ideal pango (HTML-like) markup tags.
  regex = r'((?P<reset>\x0f)|(?P<underline>\x1f)|(?P<bold>\x02)|(?P<italic>\x1d)|(?P<reverse>\x12)|(?P<color>\x03(?P<fg>\d{1,2})?(,(?P<bg>\d{1,2}))?))+'
  markup_transform = MarkupFunctor()
  msg = re.sub(regex, markup_transform, msg)
  if markup_transform.match_found:
    msg += '</span>'
  return msg

def to_hex(msg):
  '''
  Helper to Dump hex of message to screen
  '''
  line = decode(msg)
  print ':'.join(x.encode('hex') for x in line)
  print '----------------------------------'
  print ':'.join(unicode(x) for x in line)

