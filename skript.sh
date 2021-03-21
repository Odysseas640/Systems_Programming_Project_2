#!/bin/bash
#./skript.sh diseases countries input_dir numFilesPerDirectory numRecordsPerFile
# echo All Arguments = $2
if [[ $# != 5 ]]; then
	echo Incorrect number of arguments. Terminating.
	# echo $?
	exit 1
fi
# echo Correct number of arguments

if [ ! -e $1 ]; then
	echo Could not find diseases file.
	exit 2
fi
if [ ! -e $2 ]; then
	echo Could not find countries file.
	exit 3
fi

re='^[0-9]+$' # I took 2 lines from stackoverflow to check whether the argument is a number.
if ! [[ $4 =~ $re ]] ; then
	echo numFilesPerDirectory is not a number. Terminating.
	exit 4
fi
re='^[0-9]+$'
if ! [[ $5 =~ $re ]] ; then
	echo numRecordsPerFile is not a number. Terminating.
	exit 5
fi

mkdir -p $3
if [[ $? != 0 ]]; then
	echo Problem creating input_dir.
	exit 6
fi

# count diseases in the disease file, allocate an array and put them in there. Same with countries.
n_countries=$(wc -l < $2)
if [[ $? != 0 ]]; then
	echo Problem reading countries file. Maybe you typed a wrong directory or file name.
	exit 4
fi
# echo n_countries: $n_countries
readonly n_countries
n_diseases=$(wc -l < $1)
if [[ $? != 0 ]]; then
	echo Problem reading diseases file. Maybe you typed a wrong directory or file name.
	exit 5
fi
# echo n_diseases: $n_diseases
readonly n_diseases

for (( i = 0; i < $n_diseases; i++ )); do
	read -r line
	if [[ $? != 0 ]]; then
		echo Problem reading line from diseases file.
		exit 6
	fi
	diseases_array[i]=$line
    # echo "Text read from file: $line"
done < $1

names_array=("Alan" "Charlie" "Jake" "Berta" "Judith" "Evelyn" "Rose" "Herb" "Chelsea" "Mia" "Walden"\
	"Peter" "Lois" "Brian" "Stewie" "Chris" "Meg" "Glen" "Cleveland" "Joe" "Bonnie" "Carter" "Babs" "Consuela" "Mayor"\
	"Stan" "Francine" "Steve" "Roger" "Hailey" "Leonard" "Sheldon" "Penny" "Raj" "Howard" "Barry" "Amy" "Bernadette")
last_names_array=("Harper" "Schmidt" "Griffin" "Quagmire" "Brown" "Swanson" "Pewterschmit" "Penisberg" "Cooper" "Wolowitz"\
	"Kuthrapali" "Kripke" "Clarkson" "Hammond" "May" "Mayweather" "Green" "Geller" "Tribbiani" "Buffay" "Bing" "Bong"\
	"West" "Smith" "Roberts")
# Pewterschmit is missing a d, but it has to be 12 characters.
while IFS=$'\n' read -r line; do # I took 3 lines from stackoverflow to read the file line by line
    # echo "Text read from file: $line"
    mkdir -p $3/$line # create a folder in input_dir/ with the name of the country
    for (( i = 0; i < $4; i++ )); do # for i=0 to numFilesPerDirectory
    	#create a random date
    	# "file content" >> $3/datee
    	day=$RANDOM
    	day=$(($day%30 + 1))
    	month=$RANDOM
    	month=$(($month%12 + 1))
    	year=$RANDOM
    	year=$(($year%400 + 1821)) # Year from 1821 to 2220
    	date=""
    	date+=$day"-"$month"-"$year
    	# echo $date
    	touch $3/$line/$date

    	j=1
    	while [[ $j -le $5 ]] 
    	do
    	# for (( j = 1; j <= $5; j=j+2 )); do
    		#statements
	    	new_record=""

	    	#RANDOM ID
	    	recordID=$RANDOM
	    	recordID=$(($recordID+1)) # recordID is a number from 1 to 32768
	    	new_record+=$recordID" "

	    	#ENTER/EXIT
	    	# enter_exit=$RANDOM
	    	# if (($enter_exit%2 == 1)); then
	    	new_record+="ENTER "
	    	# else
	    	# 	new_record+="EXIT "
	    	# fi

	    	# FIRST NAME
	    	first_name_index=$RANDOM
	    	first_name_index=$(($first_name_index%${#names_array[@]}))
	    	new_record+=${names_array[$first_name_index]}" "

	    	# LAST NAME
	    	last_name_index=$RANDOM
	    	last_name_index=$(($last_name_index%${#last_names_array[@]}))
	    	new_record+=${last_names_array[$last_name_index]}" "

	    	#RANDOM DISEASE
	    	random_disease=$RANDOM
	    	random_disease=$(($random_disease%$n_diseases))
	    	new_record+=${diseases_array[random_disease]}" "

	    	#RANDOM AGE
	    	random_age=$RANDOM
	    	random_age=$(($random_age%121))
	    	new_record+=$random_age

	    	# echo $new_record
	    	echo $new_record >> $3/$line/$date
	    	j=$(( j+1 ))

	    	#FOLLOW-UP EXIT
	    	if [[ $j -le $5 ]]; then
	    		followup_exit=$RANDOM
	    		followup_exit=$(($followup_exit%100 + 1))
	    		if (( $followup_exit >= 10 )); then
	    			new_record2=""
	    			new_record2+=$recordID" EXIT "${names_array[$first_name_index]}" "${last_names_array[$last_name_index]}" "${diseases_array[random_disease]}" "$random_age
	    			echo $new_record2 >> $3/$line/$date
	    		else
	    			new_record3=""
			    	#RANDOM ID
			    	recordID3=$RANDOM
			    	recordID3=$(($recordID3+1)) # recordID is a number from 1 to 32768
			    	new_record3+=$recordID3" "
			    	#ENTER/EXIT
			    	new_record3+="EXIT "
			    	# FIRST NAME
			    	first_name_index3=$RANDOM
		    		first_name_index3=$(($first_name_index3%${#names_array[@]}))
		    		new_record3+=${names_array[$first_name_index3]}" "
			    	# LAST NAME
			    	last_name_index3=$RANDOM
			    	last_name_index3=$(($last_name_index%${#last_names_array[@]}))
			    	new_record3+=${last_names_array[$last_name_index3]}" "
		    		#RANDOM DISEASE
		    		random_disease3=$RANDOM
		    		random_disease3=$(($random_disease3%$n_diseases))
		    		new_record3+=${diseases_array[random_disease3]}" "
			    	#RANDOM AGE
			    	random_age3=$RANDOM
			    	random_age3=$(($random_age3%121))
			    	new_record3+=$random_age3
	    			echo $new_record3 >> $3/$line/$date
	    		fi
	    	fi
	    	j=$(( j+1 ))
    	done
    done
done < $2

# for every country
#	create a folder in input_dir/ with the name of the country
#	for i=0 i<numFilesPerDirectory
#		create a file named a random date
#		for j=0 j<numRecordsPerFile
#			create a line (random id) (enter/exit) (random name) (random disease from disease array) (random age)
#			and save it in current date file

# /input_dir/China/17-10-2019
# 889 ENTER Mary Smith COVID-2019 23