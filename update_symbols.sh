REVIEW_FILE=flyingbergman-review-raw.csv
SYMBOLS_FILE=flyingbergman.csv
rm $REVIEW_FILE 
arm-none-eabi-nm -S $1 | grep -E "^[[:alnum:]]{8}\s+[[:alnum:]]{8}\s+.*$" |
while read line
do
	ADDR=$(echo $line | sed -r 's/^([[:digit:]a-fA-F]+).*/\1/g')
	echo $line $(arm-none-eabi-addr2line -e theboss/build_dir/flyingbergman/firmware/src/theboss $ADDR) >> $REVIEW_FILE
done
cat $REVIEW_FILE | sort -k4 | awk '{print "=\"" $2 "\"," $3 "," $4}' > $SYMBOLS_FILE
cat $REVIEW_FILE | 
	grep -v "freertos/kernel" |
	grep -v "_find_by_ref" |
	grep -v "_find_by_node" |
	grep -v "_device_init" |
	grep -v "_device_register" |
	grep -v "?$" |
	grep -v "startup_stm32" |
	grep -v "stdperiph" |
	grep -v "libfdt" |
	grep -v "_ko " |
	column -t |
	sort -k5 > $REVIEW_FILE.new
mv $REVIEW_FILE.new $REVIEW_FILE
