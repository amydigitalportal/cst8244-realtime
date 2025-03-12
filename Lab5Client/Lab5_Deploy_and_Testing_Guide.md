# Lab 5 Deployment & Testing

## Deploying Server and Client programs to the QNX target

1. At the top of Momentics in the 'Launch Configuration' dropdown, select your Server project, then press the Edit (gear icon).
2. Go to "Upload" tab. Ensure the remote directory is `/tmp`.
3. Uncheck the "Remove uploaded components after session" option at the bottom.
4. Hit OK to save.
5. Run the program, then kill the program (you can stop from Momentics or `kill <pid>` from QNX environment.
6. Repeat same steps for the Client project. 

Both executables should now exist in the QNX target's `/tmp` folder.

To verify:
- In Momentics, switch to the 'QNX System Information' perspective. 
- Under the "Target File System Navigator" tab, locate the '/tmp' folder of your VM.

---
## Acceptance Test

1. Rename the server (optional) and client (required) programs. You can do this from the Momentics navigator, or via Terminal while in `/tmp`:
```
mv <your_server_exec> calc_server

mv <your_client_exec> calc_client
```

2. Place the `acceptance_test.sh.txt` file in the same directory as the `calc_client` and `calc_server` binaries. You can drag files from your Windows Explorer directly into Momentics' "Target File System Navigator" - but make sure you have the `/tmp` folder selected and that you drop into the right-side list-view.

3. Open a Terminal (SSH or VM) and navigate to the folder. Verify the contents.
```
cd /tmp

ls
```

3. Create SHELL script from TXT file, and Remove Carriage Return Characters
```
sed 's/\r$//g' ./acceptance_test.sh.txt > acceptance_test.sh
```

4. Make the Script executable
```
chmod +x acceptance_test.sh
```

5. Run the Server (you can do this from Momentics) or in Terminal (it will be trapped while the Server runs and you need to open a new Terminal instance to run commands in parallel):
```
./calc_server
```

6. Check Server PID with `pidin`.

7. Run the Acceptance Test shell script from Terminal.
```
./acceptance_test.sh <server_pid>
```