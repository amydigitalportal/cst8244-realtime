#!/bin/sh

PROGRAM_NAME="a2_metronome"

prompt_to_execute() {
	if [ $INTERACTIVE -eq 1 ]; then
		printf "\n  [ Press ENTER to execute test... ] "
		read dummy
	fi
}

run_test() {
	case "$1" in
	A)
		echo "\nUnit Test A: ./${PROGRAM_NAME}"
		echo "- Expected: usage message"
		prompt_to_execute
		./${PROGRAM_NAME}
		;;

	B)
		echo "\nUnit Test B: ./${PROGRAM_NAME} 120 2 4"
		echo "- Expected: 1 measure per second. I will use this unit-test to verify the correct cadence of your metronome."
		prompt_to_execute
		./${PROGRAM_NAME} 120 2 4 &
		;;

	C)
		echo "\nUnit Test C: cat /dev/local/metronome"
		echo "- Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
		prompt_to_execute
		cat /dev/local/metronome
		;;

	D)
		echo "\nUnit Test D: cat /dev/local/metronome-help"
		echo "- Expected: information regarding the metronome resmgr’s API"
		prompt_to_execute
		cat /dev/local/metronome-help
		;;

	E)
		echo "\nUnit Test E: echo set 100 2 4 > /dev/local/metronome"
		echo "- Expected: changes to 100 bpm in 2/4 time, runtime changes visible"
		prompt_to_execute
		echo set 100 2 4 > /dev/local/metronome
		;;

	F)
		echo "\nUnit Test F: cat /dev/local/metronome"
		echo "- Expected: [metronome: 100 beats/min, time signature 2/4, secs-per-interval: 0.30, nanoSecs: 300000000]"
		prompt_to_execute
		cat /dev/local/metronome
		;;

	G)
		echo "\nUnit Test G: echo set 200 5 4 > /dev/local/metronome"
		echo "- Expected: changes to 200 bpm in 5/4 time, runtime changes visible"
		prompt_to_execute
		echo set 200 5 4 > /dev/local/metronome
		;;

	H)
		echo "\nUnit Test H: cat /dev/local/metronome"
		echo "- Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
		prompt_to_execute
		cat /dev/local/metronome
		;;

	I)
		echo "\nUnit Test I: echo stop > /dev/local/metronome"
		echo "- Expected: metronome stops; process remains running"
		prompt_to_execute
		echo stop > /dev/local/metronome
		sleep 1
		pidin | grep metronome
		;;

	J)
		echo "\nUnit Test J: echo start > /dev/local/metronome"
		echo "- Expected: metronome resumes at 200 bpm in 5/4 time"
		prompt_to_execute
		echo start > /dev/local/metronome
		;;

	K)
		echo "\nUnit Test K: cat /dev/local/metronome"
		echo "- Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
		prompt_to_execute
		cat /dev/local/metronome
		;;

	L)
		echo "\nUnit Test L: echo stop > /dev/local/metronome"
		echo "- Expected: metronome stops again"
		prompt_to_execute
		echo stop > /dev/local/metronome
		sleep 1
		pidin | grep metronome
		;;

	M)
		echo "\nUnit Test M: echo stop > /dev/local/metronome (again)"
		echo "- Expected: no crash, still stopped"
		prompt_to_execute
		echo stop > /dev/local/metronome
		sleep 1
		pidin | grep metronome
		;;

	N)
		echo "\nUnit Test N: echo start > /dev/local/metronome"
		echo "- Expected: resumes metronome at 200 bpm in 5/4 time"
		prompt_to_execute
		echo start > /dev/local/metronome
		sleep 1
		pidin | grep metronome
		;;

	O)
		echo "\nUnit Test O: echo start > /dev/local/metronome (again)"
		echo "- Expected: metronome continues running; no change"
		prompt_to_execute
		echo start > /dev/local/metronome
		sleep 1
		pidin | grep metronome
		;;

	P)
		echo "\nUnit Test P: cat /dev/local/metronome"
		echo "- Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
		prompt_to_execute
		cat /dev/local/metronome
		;;

	Q)
		echo "\nUnit Test Q: echo pause 3 > /dev/local/metronome"
		echo "- Expected: metronome pauses 3 sec mid-measure, resumes on next beat"
		prompt_to_execute
		echo pause 3 > /dev/local/metronome
		;;

	R)
		echo "\nUnit Test R: echo pause 10 > /dev/local/metronome"
		echo "- Expected: error message due to out-of-bounds pause value"
		prompt_to_execute
		echo pause 10 > /dev/local/metronome
		;;

	S)
		echo "\nUnit Test S: echo bogus > /dev/local/metronome"
		echo "- Expected: error message for unknown command"
		prompt_to_execute
		echo bogus > /dev/local/metronome
		;;

	T)
		echo "\nUnit Test T: echo set 120 2 4 > /dev/local/metronome"
		echo "- Expected: cadence returns to 1 measure per second"
		prompt_to_execute
		echo set 120 2 4 > /dev/local/metronome
		;;

	U)
		echo "\nUnit Test U: cat /dev/local/metronome"
		echo "- Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
		prompt_to_execute
		cat /dev/local/metronome
		;;

	V)
		echo "\nUnit Test V: cat /dev/local/metronome-help"
		echo "- Expected: information regarding the metronome resmgr’s API"
		prompt_to_execute
		cat /dev/local/metronome-help
		;;

	W)
		echo "\nUnit Test W: echo Writes-Not-Allowed > /dev/local/metronome-help"
		echo "- Expected: error message, since this is a read-only resource"
		prompt_to_execute
		echo Writes-Not-Allowed > /dev/local/metronome-help
		;;

	X)
		echo "\nUnit Test X: echo quit > /dev/local/metronome && pidin | grep metronome"
		echo "- Expected: metronome gracefully terminates"
		prompt_to_execute
		echo quit > /dev/local/metronome
		sleep 1
		pidin | grep metronome
		;;
	esac
}


# ─────────────────────────────
# Determine if we're in interactive mode
INTERACTIVE=0
SINGLE_TEST=""

for arg in "$@"; do
	if [ "$arg" = "--interactive" ]; then
		INTERACTIVE=1
	else
		SINGLE_TEST="$arg"
	fi
done

# ─────────────────────────────
# Single test mode (e.g., ./acceptance-test.sh [--interactive] G)
if [ -n "$SINGLE_TEST" ]; then
	INPUT_UPPER=$(echo "$SINGLE_TEST" | tr '[:lower:]' '[:upper:]')
	echo "\nRunning Unit Test $INPUT_UPPER..."
	run_test "$INPUT_UPPER"
	echo "\nSingle test $INPUT_UPPER completed.\n"
	exit 0
fi

# ─────────────────────────────
# Automated test sequence (C–X, skipping A & B)
echo "\nRunning automated acceptance test sequence..."
[ $INTERACTIVE -eq 1 ] && echo "Mode: INTERACTIVE (press enter to proceed between tests)"
echo ""

# Utility: advance by sleep or user key
advance() {
	if [ $INTERACTIVE -eq 1 ]; then
		printf "\n--- Test complete! ---\n  [ Press ENTER to continue to next test... ] "
		read dummy
	else
		sleep "$1"
	fi
}

# Run tests
run_test C; advance 3
run_test D; advance 3
run_test E; advance 5
run_test F; advance 3
run_test G; advance 5
run_test H; advance 3
run_test I; advance 3
run_test J; advance 5
run_test K; advance 3
run_test L; advance 3
run_test M; advance 3
run_test N; advance 3
run_test O; advance 5
run_test P; advance 3
run_test Q; advance 5
run_test R; advance 3
run_test S; advance 3
run_test T; advance 5
run_test U; advance 3
run_test V; advance 3
run_test W; advance 3
run_test X; advance 3

echo "\nAcceptance test completed.\n"
exit 0
