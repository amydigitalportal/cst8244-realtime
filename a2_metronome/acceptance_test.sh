#!/bin/sh

PROGRAM_NAME="a2_metronome"

# echo "\n"
# echo "Unit Test A: ./metronome"
# echo "Expected: usage message"
# ./${PROGRAM_NAME}
# echo "\n"
# sleep 3

# echo "\n"
# echo "Unit Test B: ./metronome 120 2 4"
# echo "Expected: metronome ticks at 1 measure per second (observe for 3–5 seconds)"
# ./${PROGRAM_NAME} 120 2 4 &
# echo "\n"
# sleep 5

echo "\n"
echo "Unit Test C: cat /dev/local/metronome"
cat /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test D: cat /dev/local/metronome-help"
cat /dev/local/metronome-help
echo "\n"
sleep 3

echo "\n"
echo "Unit Test E: echo set 100 2 4 > /dev/local/metronome"
echo "Expected: change to 100 bpm 2/4"
echo set 100 2 4 > /dev/local/metronome
echo "\n"
sleep 5

echo "\n"
echo "Unit Test F: cat /dev/local/metronome"
cat /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test G: echo set 200 5 4 > /dev/local/metronome"
echo "Expected: change to 200 bpm 5/4"
echo set 200 5 4 > /dev/local/metronome
echo "\n"
sleep 5

echo "\n"
echo "Unit Test H: cat /dev/local/metronome"
cat /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test I: echo stop > /dev/local/metronome"
echo "Expected: metronome stops (verify visually), but process still running:"
echo stop > /dev/local/metronome
pidin | grep metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test J: echo start > /dev/local/metronome"
echo "Expected: metronome resumes at 200 bpm 5/4"
echo start > /dev/local/metronome
echo "\n"
sleep 5

echo "\n"
echo "Unit Test K: cat /dev/local/metronome"
cat /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test L: echo stop > /dev/local/metronome"
echo "Expected: metronome stops"
echo stop > /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test M: echo stop > /dev/local/metronome (again)"
echo stop > /dev/local/metronome
echo "Expected: still stopped, no crash"
echo "\n"
sleep 3

echo "\n"
echo "Unit Test N: echo start > /dev/local/metronome"
echo "Expected: resumes at 200 bpm 5/4"
echo start > /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test O: echo start > /dev/local/metronome (again)"
echo "Expected: still running at 200 bpm 5/4"
echo "\n"
echo start > /dev/local/metronome
sleep 5

echo "\n"
echo "Unit Test P: cat /dev/local/metronome"
cat /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test Q: echo pause 3 > /dev/local/metronome"
echo pause 3 > /dev/local/metronome
echo "Expected: pause for 3 seconds mid-measure, then resume"
echo "\n"
sleep 5

echo "\n"
echo "Unit Test R: echo pause 10 > /dev/local/metronome"
echo pause 10 > /dev/local/metronome
echo "Expected: error message (10 is out of bounds)"
echo "\n"
sleep 3

echo "\n"
echo "Unit Test S: echo bogus > /dev/local/metronome"
echo "Expected: error message for unknown command"
echo bogus > /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test T: echo set 120 2 4 > /dev/local/metronome"
echo set 120 2 4 > /dev/local/metronome
echo "Expected: 1 measure per second cadence"
sleep 5
echo "\n"

echo "\n"
echo "Unit Test U: cat /dev/local/metronome"
cat /dev/local/metronome
echo "\n"
sleep 3

echo "\n"
echo "Unit Test V: cat /dev/local/metronome-help"
cat /dev/local/metronome-help
echo "\n"
sleep 3

echo "\n"
echo "Unit Test W: echo Writes-Not-Allowed > /dev/local/metronome-help"
echo Writes-Not-Allowed > /dev/local/metronome-help
echo "Expected: error message (read-only resource)"
echo "\n"
sleep 3

echo "\n"
echo "Unit Test X: echo quit > /dev/local/metronome && pidin | grep metronome"
echo "Expected: metronome process terminated"
echo quit > /dev/local/metronome
sleep 1
pidin | grep metronome
echo "\n"
sleep 3

echo "\n"
echo "Acceptance test completed."
echo "\n"
exit 0
