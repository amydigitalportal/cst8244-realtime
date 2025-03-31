#!/bin/sh

PROGRAM_NAME="a2_metronome"

run_test() {
	case "$1" in
	A)
		echo "\nUnit Test A: ./${PROGRAM_NAME}"
		echo "Expected: usage message"
        echo "\n"
		./${PROGRAM_NAME}
		;;

	B)
		echo "\nUnit Test B: ./${PROGRAM_NAME} 120 2 4"
		echo "Expected: 1 measure per second. I will use this unit-test to verify the correct cadence of your metronome. Let your metronome run for 3 to 5 seconds so we can observe the metronome’s run-time behaviour."
		./${PROGRAM_NAME} 120 2 4 &
		;;

	C)
		echo "\nUnit Test C: cat /dev/local/metronome"
		echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
		echo "\n"
		cat /dev/local/metronome
		;;

	D)
		echo "\nUnit Test D: cat /dev/local/metronome-help"
		echo "Expected: information regarding the metronome resmgr’s API, as seen above."
		echo "\n"
		cat /dev/local/metronome-help
		;;

	E)
		echo "\nUnit Test E: echo set 100 2 4 > /dev/local/metronome"
		echo "Expected: metronome regmgr changes settings to: 100 bpm in 2/4 time; run-time behaviour of metronome changes to 100 bpm in 2/4 time. Let your metronome run for 3 to 5 seconds so we can observe the metronome’s run-time behaviour."
		echo set 100 2 4 > /dev/local/metronome
		;;

	F)
		echo "\nUnit Test F: cat /dev/local/metronome"
		echo "Expected: [metronome: 100 beats/min, time signature 2/4, secs-per-interval: 0.30, nanoSecs: 300000000]"
		echo "\n"
		cat /dev/local/metronome
		;;

	G)
		echo "\nUnit Test G: echo set 200 5 4 > /dev/local/metronome"
		echo "Expected: metronome regmgr changes settings to: 200 bpm in 5/4 time; run-time behaviour of metronome changes to 200 bpm in 5/4 time. Let your metronome run for 3 to 5 seconds so we can observe the metronome’s run-time behaviour."
		echo set 200 5 4 > /dev/local/metronome
		;;

	H)
		echo "\nUnit Test H: cat /dev/local/metronome"
		echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
		echo "\n"
		cat /dev/local/metronome
		;;

	I)
		echo "\nUnit Test I: echo stop > /dev/local/metronome"
		echo "Expected: metronome stops running; metronome resmgr is still running as a process: pidin | grep metronome."
		echo stop > /dev/local/metronome
		sleep 1
		pidin | grep metronome
		;;

	J)
		echo "\nUnit Test J: echo start > /dev/local/metronome"
		echo "Expected: metronome starts running again at 200 bpm in 5/4 time, which is the last setting; metronome resmgr is still running as a process: pidin | grep metronome. Let your metronome run for 3 to 5 seconds so we can observe the metronome’s run-time behaviour."
		echo start > /dev/local/metronome
		;;

	K)
		echo "\nUnit Test K: cat /dev/local/metronome"
		echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
		echo "\n"
		cat /dev/local/metronome
		;;

	L)
		echo "\nUnit Test L: echo stop > /dev/local/metronome"
		echo "Expected: metronome stops running; metronome resmgr is still running as a process: pidin | grep metronome."
		sleep 1
		pidin | grep metronome
        echo stop > /dev/local/metronome
		;;

	M)
		echo "\nUnit Test M: echo stop > /dev/local/metronome (again)"
		echo "Expected: metronome remains stopped; metronome resmgr is still running as a process: pidin | grep metronome."
		sleep 1
		pidin | grep metronome
        echo stop > /dev/local/metronome
		;;

	N)
		echo "\nUnit Test N: echo start > /dev/local/metronome"
		echo "Expected: metronome starts running again at 200 bpm in 5/4 time, which is the last setting; metronome resmgr is still running as a process: pidin | grep metronome. Let your metronome run for 3 to 5 seconds so we can observe the metronome’s run-time behaviour."
		sleep 1
        pidin | grep metronome
        echo start > /dev/local/metronome
		;;

	O)
		echo "\nUnit Test O: echo start > /dev/local/metronome (again)"
		echo "Expected: metronome is still running again at 200 bpm in 5/4 time, which is the last setting; metronome resmgr is still running as a process: pidin | grep metronome. Let your metronome run for 3 to 5 seconds so we can observe the metronome’s run-time behaviour."
		sleep 1
        pidin | grep metronome
        echo start > /dev/local/metronome
		;;

	P)
		echo "\nUnit Test P: cat /dev/local/metronome"
		echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
		echo "\n"
		cat /dev/local/metronome
		;;

	Q)
		echo "\nUnit Test Q: echo pause 3 > /dev/local/metronome"
		echo "Expected: metronome continues on next beat (not next measure). It’s your burden to pause the metronome mid-measure (i.e. repeat until expected behaviour) You can be called upon to pause your metronome at any (i.e. random) point during the demo."
		echo pause 3 > /dev/local/metronome
		;;

	R)
		echo "\nUnit Test R: echo pause 10 > /dev/local/metronome"
		echo "Expected: properly formatted error message, and metronome continues to run."
		echo pause 10 > /dev/local/metronome
		;;

	S)
		echo "\nUnit Test S: echo bogus > /dev/local/metronome"
		echo "Expected: properly formatted error message, and metronome continues to run."
		echo bogus > /dev/local/metronome
		;;

	T)
		echo "\nUnit Test T: echo set 120 2 4 > /dev/local/metronome"
		echo "Expected: 1 measure per second. I will use this unit-test to verify the correct cadence of your metronome. Let your metronome run for 3 to 5 seconds so we can observe the metronome’s run-time behaviour."
		echo set 120 2 4 > /dev/local/metronome
		;;

	U)
		echo "\nUnit Test U: cat /dev/local/metronome"
		echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
		cat /dev/local/metronome
		;;

	V)
		echo "\nUnit Test V: cat /dev/local/metronome-help"
		echo "Expected: information regarding the metronome resmgr’s API, as seen above."
		echo "\n"
		cat /dev/local/metronome-help
		;;

	W)
		echo "\nUnit Test W: echo Writes-Not-Allowed > /dev/local/metronome-help"
		echo "Expected: properly formatted error message, and metronome continues to run."
		echo Writes-Not-Allowed > /dev/local/metronome-help
		;;

	X)
		echo "\nUnit Test X: echo quit > /dev/local/metronome && pidin | grep metronome"
		echo "Expected: metronome gracefully terminates."
		echo quit > /dev/local/metronome
		sleep 1
		pidin | grep metronome
		;;
	esac
}

# ─────────────────────────────
# Manual: Run one test
if [ $# -eq 1 ]; then
	INPUT_UPPER=$(echo "$1" | tr '[:lower:]' '[:upper:]')
	run_test "$INPUT_UPPER"
	echo "\nSingle test $INPUT_UPPER completed.\n"
	exit 0
fi


# ─────────────────────────────
# Automated sequence (no A, B)
echo "\nRunning automated acceptance test sequence...\n"

run_test C; sleep 3
run_test D; sleep 3
run_test E; sleep 5
run_test F; sleep 3
run_test G; sleep 5
run_test H; sleep 3
run_test I; sleep 3
run_test J; sleep 5
run_test K; sleep 3
run_test L; sleep 3
run_test M; sleep 3
run_test N; sleep 3
run_test O; sleep 5
run_test P; sleep 3
run_test Q; sleep 5
run_test R; sleep 3
run_test S; sleep 3
run_test T; sleep 5
run_test U; sleep 3
run_test V; sleep 3
run_test W; sleep 3
run_test X; sleep 3

echo "\nAcceptance test completed.\n"
exit 0
