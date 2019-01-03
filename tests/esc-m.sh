echo -en "\x1b""[2J"
echo -ne "\x1b[3;1H"
echo -en "FOO"
echo -en "\x1b""M"
echo -en "BAR"
sleep 60

