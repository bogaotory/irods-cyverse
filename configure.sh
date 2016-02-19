#!/usr/bin/env bash
set -a
. env.properties

function fillConfig() {
	FILES=$1/*.template
	for f in $FILES
	do
		# echo "$(basename $f .template)"
		if [ -f $1/"$(basename $f .template)" ]; then
			cp $1/"$(basename $f .template)" $1/"$(basename $f .template)".bak
			rm $1/"$(basename $f .template)"
		fi

		while IFS='' read -r line || [[ -n "$line" ]]; do
			if [[ ! "$line" =~ ^\# ]]; then
				# eval "echo -e "$line"" 
				eval "echo -e "$line"" >> $1/"$(basename $f .template)"
			else
				echo "$line" >> $1/"$(basename $f .template)"
			fi
		done < $f

		cat $1/"$(basename $f .template)"
	done
}

fillConfig ./irods-3.3.1-cyverse/config
fillConfig ./client/config