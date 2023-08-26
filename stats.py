###!/usr/bin/python3

import sys
import os
import math
from ftplib import FTP
import re
from config import FTP_HOST, FTP_USER, FTP_PASSWD

# This script is used to get the stats files each week.


SEASON = "Reg"
#SEASON = "Ps"

YEAR = 2023
STATS_DIR = os.path.join(os.getcwd(), "stats")

def print_help():

	print("Syntax error:")
	print("\t%s: <year (like 2018)> <week number (like 5)>" % sys.argv[0])
	print("\n")
	sys.exit(1)

def make_dir(local_dir_name):

	if not os.path.exists(local_dir_name):
            os.makedirs(local_dir_name)

def get_stat_files(year, week):

   ftp = FTP(FTP_HOST, FTP_USER, FTP_PASSWD)

   ftp.cwd("/%d/%s/Week%02d" % (year, SEASON, week))
   #ftp.cwd("/%d/PS/Week%02d" % (year, week))
   filenames = ftp.nlst()

   count = 0
   for filename in filenames:
      sys.stdout.write(".")
      if(count > 0 and count%10 == 0):
         sys.stdout.write("\n")
         sys.stdout.flush()
         file = open(filename, 'wb')
         ftp.retrbinary('RETR '+ filename, file.write)
         file.close()
         count = count + 1

   sys.stdout.write("\n")
   sys.stdout.flush()

   ftp.cwd("/%d/RTF/%s/Week%02d" % (year, SEASON, week))
   filenames = ftp.nlst()

   for filename in filenames:
      sys.stdout.write(".")
      if(count > 0 and count%10 == 0):
            sys.stdout.write("\n")
      sys.stdout.flush()
      file = open(filename, 'wb')
      ftp.retrbinary('RETR '+ filename, file.write)
      file.close()
      count = count + 1


   print("\n%d files retrieved" % count)
		
def striprtf(text):
   pattern = re.compile(r"\\([a-z]{1,32})(-?\d{1,10})?[ ]?|\\'([0-9a-f]{2})|\\([^a-z])|([{}])|[\r\n]+|(.)", re.I)
   # control words which specify a "destionation".
   #destinations = frozenset((
   destinations = [
      'aftncn','aftnsep','aftnsepc','annotation','atnauthor','atndate','atnicn','atnid',
      'atnparent','atnref','atntime','atrfend','atrfstart','author','background',
      'bkmkend','bkmkstart','blipuid','buptim','category','colorschememapping',
      'colortbl','comment','company','creatim','datafield','datastore','defchp','defpap',
      'do','doccomm','docvar','dptxbxtext','ebcend','ebcstart','factoidname','falt',
      'fchars','ffdeftext','ffentrymcr','ffexitmcr','ffformat','ffhelptext','ffl',
      'ffname','ffstattext','field','file','filetbl','fldinst','fldrslt','fldtype',
      'fname','fontemb','fontfile','fonttbl','footer','footerf','footerl','footerr',
      'footnote','formfield','ftncn','ftnsep','ftnsepc','g','generator','gridtbl',
      'header','headerf','headerl','headerr','hl','hlfr','hlinkbase','hlloc','hlsrc',
      'hsv','htmltag','info','keycode','keywords','latentstyles','lchars','levelnumbers',
      'leveltext','lfolevel','linkval','list','listlevel','listname','listoverride',
      'listoverridetable','listpicture','liststylename','listtable','listtext',
      'lsdlockedexcept','macc','maccPr','mailmerge','maln','malnScr','manager','margPr',
      'mbar','mbarPr','mbaseJc','mbegChr','mborderBox','mborderBoxPr','mbox','mboxPr',
      'mchr','mcount','mctrlPr','md','mdeg','mdegHide','mden','mdiff','mdPr','me',
      'mendChr','meqArr','meqArrPr','mf','mfName','mfPr','mfunc','mfuncPr','mgroupChr',
      'mgroupChrPr','mgrow','mhideBot','mhideLeft','mhideRight','mhideTop','mhtmltag',
      'mlim','mlimloc','mlimlow','mlimlowPr','mlimupp','mlimuppPr','mm','mmaddfieldname',
      'mmath','mmathPict','mmathPr','mmaxdist','mmc','mmcJc','mmconnectstr',
      'mmconnectstrdata','mmcPr','mmcs','mmdatasource','mmheadersource','mmmailsubject',
      'mmodso','mmodsofilter','mmodsofldmpdata','mmodsomappedname','mmodsoname',
      'mmodsorecipdata','mmodsosort','mmodsosrc','mmodsotable','mmodsoudl',
      'mmodsoudldata','mmodsouniquetag','mmPr','mmquery','mmr','mnary','mnaryPr',
      'mnoBreak','mnum','mobjDist','moMath','moMathPara','moMathParaPr','mopEmu',
      'mphant','mphantPr','mplcHide','mpos','mr','mrad','mradPr','mrPr','msepChr',
      'mshow','mshp','msPre','msPrePr','msSub','msSubPr','msSubSup','msSubSupPr','msSup',
      'msSupPr','mstrikeBLTR','mstrikeH','mstrikeTLBR','mstrikeV','msub','msubHide',
      'msup','msupHide','mtransp','mtype','mvertJc','mvfmf','mvfml','mvtof','mvtol',
      'mzeroAsc','mzeroDesc','mzeroWid','nesttableprops','nextfile','nonesttables',
      'objalias','objclass','objdata','object','objname','objsect','objtime','oldcprops',
      'oldpprops','oldsprops','oldtprops','oleclsid','operator','panose','password',
      'passwordhash','pgp','pgptbl','picprop','pict','pn','pnseclvl','pntext','pntxta',
      'pntxtb','printim','private','propname','protend','protstart','protusertbl','pxe',
      'result','revtbl','revtim','rsidtbl','rxe','shp','shpgrp','shpinst',
      'shppict','shprslt','shptxt','sn','sp','staticval','stylesheet','subject','sv',
      'svb','tc','template','themedata','title','txe','ud','upr','userprops',
      'wgrffmtfilter','windowcaption','writereservation','writereservhash','xe','xform',
      'xmlattrname','xmlattrvalue','xmlclose','xmlname','xmlnstbl',
      'xmlopen'
	]
   #))
   # Translation of some special characters.
   specialchars = {
      'par': '\n',
      'sect': '\n\n',
      'page': '\n\n',
      'line': '\n',
      'tab': '\t',
      'emdash': '\u2014',
      'endash': '\u2013',
      'emspace': '\u2003',
      'enspace': '\u2002',
      'qmspace': '\u2005',
      'bullet': '\u2022',
      'lquote': '\u2018',
      'rquote': '\u2019',
      'ldblquote': '\201C',
      'rdblquote': '\u201D',
   }
   stack = []
   ignorable = False       # Whether this group (and all inside it) are "ignorable".
   ucskip = 1              # Number of ASCII characters to skip after a unicode character.
   curskip = 0             # Number of ASCII characters left to skip
   out = []                # Output buffer.
   #for match in pattern.finditer(text.decode()):
   for match in pattern.finditer(text):
      word,arg,hex,char,brace,tchar = match.groups()
      if brace:
         curskip = 0
         if brace == '{':
            # Push state
            stack.append((ucskip,ignorable))
         elif brace == '}':
            # Pop state
            ucskip,ignorable = stack.pop()
      elif char: # \x (not a letter)
         curskip = 0
         if char == '~':
            if not ignorable:
                out.append('\xA0')
         elif char in '{}\\':
            if not ignorable:
               out.append(char)
         elif char == '*':
            ignorable = True
      elif word: # \foo
         curskip = 0
         if word in destinations:
            ignorable = True
         elif ignorable:
            pass
         elif word in specialchars:
            out.append(specialchars[word])
         elif word == 'uc':
            ucskip = int(arg)
         elif word == 'u':
            c = int(arg)
            if c < 0: c += 0x10000
            if c > 127: out.append(chr(c)) #NOQA
            else: out.append(chr(c))
            curskip = ucskip
      elif hex: # \'xx
         if curskip > 0:
            curskip -= 1
         elif not ignorable:
            c = int(hex,16)
            if c > 127: out.append(chr(c)) #NOQA
            else: out.append(chr(c))
      elif tchar:
         if curskip > 0:
            curskip -= 1
         elif not ignorable:
            out.append(tchar)
   return ''.join(out)

def entire_stats_rtf_to_txt(year, week):

	text = open('entirestats.rtf', 'r').read()
	text = striprtf(text)
	file = open("entirestats.txt", "w")
	file.write(text)
	file.close()

	

if(len(sys.argv) <= 2):
	print_help()

year = int(sys.argv[1])
week = int(sys.argv[2])

print("Processing for week=%s, year=%s" % (week, year))

make_dir('%s' % STATS_DIR)
make_dir('%s/%d' % (STATS_DIR, year))
make_dir('%s/%d/Week%02d' % (STATS_DIR, year, week))

#week_dir = os.path.expanduser("%s/%s/Week%02d" % (STATS_DIR, year, int(week)))

#week_dir = '%s/%s/Week%02d' % (STATS_DIR, year, int(week))


os.chdir(os.path.expanduser("%s/%d/Week%02d" % (STATS_DIR, year, week)))
week_dir = os.getcwd()
print("Output directory will be %s" % week_dir)

get_stat_files(year, week)

entire_stats_rtf_to_txt(year, week)





