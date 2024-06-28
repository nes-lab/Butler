#!/bin/bash
#
# embeds an exe/elf file into a the FlockLab XML config file
#
# 2020, rdaforno
# 
#
# in order to use "drag&drop" to pass the image file and XML config to this script,
# you will need to create a *.desktop file with the following content:
#    [Desktop Entry]
#    Name=Embed FlockLab Image Wrapper
#    Comment=Drop FlockLab XML config and target image file here
#    Exec=[absolute_path_to_embed_image_script] %U
#    Type=Application
#

SEDCMD=sed
B64CMD=base64
XMLFILE=flocklab.xml     # default file name, if not provided via argument

# check if sed tool is installed
which $SEDCMD > /dev/null 2>&1
if [ $? -ne 0 ]
then
  echo "command '$SEDCMD' not found"
  exit 1
fi

# check if base64 tool is installed
which $B64CMD > /dev/null 2>&1
if [ $? -ne 0 ]
then
  echo "command '$B64CMD' not found"
  exit 1
fi

# at least one arguments are required (the target image)
if [ $# -lt 1 ]
then
  echo "usage: $0 [image file (exe/elf)] ([input / output XML file])"
  exit 1
fi

# if an additional argument is provided, check if it is an xml file
IMGFILE=$1
if [ $# -gt 1 ]
then
  XMLFILE=$2
  if [[ $1 == *.xml ]]
  then
    # swap the two files
    XMLFILE=$1
    IMGFILE=$2
  fi
fi

# check file extension of image
if [[ ! $IMGFILE == *.exe ]] && [[ ! $IMGFILE == *.elf ]] && [[ ! $IMGFILE == *.hex ]] && [[ ! $IMGFILE == *.sky ]] && [[ ! $IMGFILE == *.out ]]
then
  echo "invalid image file format"
  exit 2
fi

# check if files exist
if [ ! -f $IMGFILE ]
then
  echo "file $IMGFILE not found"
  exit 3
fi
if [ ! -f $XMLFILE ]
then
  echo "file $XMLFILE not found"
  exit 4
fi

if [ ! -f $XMLFILE ]
then
  echo "file $XMLFILE not found"
  exit 5
fi

B64FILE="$IMGFILE.b64"

# convert to base 64
$B64CMD $IMGFILE > $B64FILE
# insert binary into xml (in-place)
$SEDCMD -i -n '1h;1!H;${ g;s/<data>.*<\/data>/<data>\n<\/data>/;p}' $XMLFILE
$SEDCMD -i "/<data>/r ${B64FILE}" $XMLFILE
# remove temporary file
rm $B64FILE

echo "image $IMGFILE embedded into $XMLFILE"
