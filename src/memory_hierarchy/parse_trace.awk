#!/usr/bin/gawk -f
BEGIN {
    OFS="\t"
    state1=0
    state2=0
    state3=0
    state4=0
}
$3~/start/ {
    if ($2 == 0) {
	state1=1
    }
    else if ($2 == 1) {
	state2=1;
    }
    else if ($2 == 2) {
	state3=1;
    }
    else {
	state4=1;
    }

}
$3~/end/ {
    if ($2 == 0) {
	state1=0;
    }
    else if ($2 == 1) {
	state2=0;
    }
    else if ($2 == 2) {
	state3=0;
    }
    else {
	state4=0;
    }
}
$3~/m/ && NF==6 {
    if ($2==0 && state1==1)
	print $4, $5 >> "thread_1.out"
    if ($2==1 && state2==1)
	print $4, $5 >> "thread_2.out"
    if ($2==2 && state3==1)
	print $4, $5 >> "thread_3.out"
    if ($2==3 && state4==1)
	print $4, $5 >> "thread_4.out"
}
