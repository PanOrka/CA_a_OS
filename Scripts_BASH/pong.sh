board1=("█" "█" "█" "█")
board2=("█" "█" "█" "█")

w=$(tput cols)
h=$(tput lines)
whitespace=" "

startx=`echo "$w/2" | bc`
starty=`echo "$h/2" | bc`
ballx=$startx
bally=$starty
vel_x=1
vel_y=-1
max_speed=2

pos1x=3
pos2x=$(( $w - 2 ))
pos1y=$(( $h-5 ))
pos2y=$(( $h-5 ))
len=4

draw_board1() {
  erase=$pos1y
  for ((i=0; i<len; i++)) {
    echo -en "\033[${erase};${pos1x}H$whitespace"
    erase=$(( $erase-1 ))
  }
  (( pos1y += $1 ))
  (( pos1y = pos1y > h ? h : pos1y < len ? len : pos1y  ))
  hei=$pos1y
  for ((i=0; i<len; i++)); do
    echo -en "\033[${hei};${pos1x}H$board1"
    hei=$(( $hei-1 ))
  done
}

draw_board2() {
  erase=$pos2y
  for ((i=0; i<len; i++)) {
    echo -en "\033[${erase};${pos2x}H$whitespace"
    erase=$(( $erase-1 ))
  }
  (( pos2y += $1 ))
  (( pos2y = pos2y > h ? h : pos2y < len ? len : pos2y  ))
  hei=$pos2y
  for ((i=0; i<len; i++)); do
    echo -en "\033[${hei};${pos2x}H$board2"
    hei=$(( $hei-1 ))
  done
}
tput civis
tput clear

draw_board1 1;
draw_board2 1;

while [[ $q != q ]]; do
  echo -en "\033[${bally};${ballx}H "
  (( ballx += vel_x ))
  (( bally += vel_y ))
  echo -en "\033[${bally};${ballx}H●"
  read -n 1 -s -t 0.05 q
  case "$q" in
    [wW] ) draw_board1 -2;;
    [sS] ) draw_board1 2;;
    [oO] ) draw_board2 -2;;
    [lL] ) draw_board2 2;;
  esac
  (( ballx >= pos2x - 1 || ballx <= pos1x + 1 )) && (( vel_x = - vel_x ))
  (( bally + vel_y < 1 )) && echo -en "\033[${bally};${ballx}H " && (( bally = 1 - bally - vel_y )) && (( vel_y = - vel_y )) && echo -en "\033[${bally};${ballx}H●"
  (( bally + vel_y > h )) && echo -en "\033[${bally};${ballx}H " && (( bally = 2*h - 1 - bally - vel_y )) && (( vel_y = - vel_y  )) && echo -en "\033[${bally};${ballx}H●"
  if (( ballx <= pos1x + 1 )); then
    if (( bally <= pos1y && bally >= pos1y - len  )); then
      (( vel_y = bally - (pos1y - len) - len/2 ))
      (( ${vel_y//-/} > max_speed )) && vel_y=${vel-y//[0-9]*/$max_speed}
    else
      echo -en "\033[${bally};${ballx}H "
      ballx=$startx
      bally=$starty
      vel_y=$(( RANDOM % max_speed + 1))
      vell_x=1
    fi
  fi
  if (( ballx >= pos2x - 1 )); then
    if (( bally <= pos2y && bally >= pos2y - len  )); then
      (( vel_y = bally - (pos2y - len) - len/2 ))
      (( ${vel_y//-/} > max_speed )) && vel_y=${vel-y//[0-9]*/$max_speed}
    else
      echo -en "\033[${bally};${ballx}H "
      ballx=$startx
      bally=$starty
      vel_y=$(( RANDOM % max_speed + 1))
      vell_x=-1
    fi
  fi
done
tput clear
tput cnorm
