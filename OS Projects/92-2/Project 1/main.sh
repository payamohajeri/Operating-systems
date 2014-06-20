#!/bin/bash
#******************************************************************************/
#**									License									 **/
#******************************************************************************/
#**
#  Mastermind sh
#  Copyright (C) 2009 Marco Antognini (hiura@romandie.com)
#  License : CC-BY-SA 3.0
#  	You can find the full legal code at
#  	http://creativecommons.org/licenses/by-sa/3.0/
#  	or in the local file cc-by-sa-3.0-legalcode.html .
#  	Here is only an abstract :
#
#  You are free :
#  	to Share — to copy, distribute and transmit the work
#	to Remix — to adapt the work
#
#  Under the following conditions :
#  	Attribution. You must attribute the work in the manner
#		specified by the author or licensor (but not
#		in any way that suggests that they endorse you
#		or your use of the work).
#	Share Alike. If you alter, transform, or build upon this
#		work, you may distribute the resulting work only
#		under the same, similar or a compatible license.
#
#  For any reuse or distribution, you must make clear to others
#  	the license terms of this work. The best way to do this
#	is with a link to this
#       	(http://creativecommons.org/licenses/by-sa/3.0/) web page.
#
#  Any of the above conditions can be waived if you get
#  	permission from the copyright holder.
#
#  Nothing in this license impairs or restricts the author's
#  	moral rights.
#
#**/

#******************************************************************************/
#**									SETUP									 **/
#******************************************************************************/

MAX_ROUND=10 ###< modifiable.

RIGHT="r" ###< modifiable.
EXIST="e" ###< modifiable.
WRONG="w" ###< modifiable.

#******************************************************************************/
#**									VOC										 **/
#******************************************************************************/
#**
#    input : user input.
#    master : solution.
#    msg : "Indicator of success".
#    round : nombre of tries done yet.
#    MAX_ROUND : number of tries allowed.
#    len : master's length.
#
#    RIGHT : in the right place.
#    EXIST : exists but in the wrong place.
#    WRONG : doesn't exist.
#*/

#******************************************************************************/
#**									FUNCTIONS								 **/
#******************************************************************************/
_initused()
{
	used=""
	i=0
	while [[ "$i" -lt "$len" ]]
	do
		used=${used}"F"
		let i++
	done
}

_updateused() # $1 : position where to replace F by T.
{
	if [[ $1 -gt $len ]] || [[ $1 -lt 0 ]]
	then
		echo "Bad parameter @ _updatefound"
		exit 2
	elif [[ $1 -eq 0 ]]
	then
		used="T"${used:($1 + 1)}
	else
		used=${used:0:($1 - 1)}"T"${used:$1}
	fi
}

_msgadd() # $1 : what to append ( RIGHT, EXIST or WRONG ) .
{
	msg=${msg}$1
}

#******************************************************************************/
#**									"MAIN"									 **/
#******************************************************************************/

## Get master
read -p "Enter master : " master
len=${#master}

colors=["orange", "blue", "green", "white", "yellow", "brown", "red", "grey"];

## Clear terminal
clear

## Start game loop
round=0 ## Current round
state="false" ## Does the user win ?
while [[ $round -lt $MAX_ROUND && "$state" = "false" ]] ## main loop
do
	echo "Still $(( $MAX_ROUND - $round )) attempt(s)"

	## Get user input
	read -p "Enter the solution ( $len characters ) : " input
	if [[ "$len" != "${#input}" ]]
	then
		echo "Bad lenght."
		continue
	fi

	## Get msg.
	msg=""
	_initused
	i=0
	while [[ "$i" -lt "$len" ]] ## loop A
	do
		if [[ "${master:$i:1}" = "${input:$i:1}" ]] ## if RIGHT
		then
			_updateused $i
			_msgadd $RIGHT
		else ## -> EXIST or WRONG
			found="false"
			j=0
			while [[ "$j" -lt "$len" ]] ## loop B
			do
				if [[ "$i" != "$j" ]] || \
				   [[ "${used:$j:1}" != "T" ]] && \
				   [[ "${input:$i:1}" = "${master:$j:1}" ]]
				then
					## Warning : EXIST only if input[j] != master[j] .
					if [[ "${input:$j:1}" != "${master:$j:1}" ]]
					then
						_updateused $j
						_msgadd $EXIST
						found="true"
					fi
				fi

				let j++
			done ## loop B

			if [[ "$found" = "false" ]]
			then
				_msgadd $WRONG
			fi
		fi ## -> EXIST or WRONG

		let i++
	done ## loop A
	if [[ "$input" = "$master" ]]
	then
		state="true"
	fi

	echo $msg

	## Update round.
	let round++
done ## main loop

if [[ "$state" = "true" ]]
then
	echo "Bravo!"
else
	echo "Boooo!"
fi

exit 0


