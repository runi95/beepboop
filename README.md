# BeepBoop

A simple bot for steamgifts.com

## Getting Started

Here's what you need to compile this program
```
myhtml - https://github.com/lexborisov/myhtml
curlpp - https://github.com/jpbarrette/curlpp
```

### Installing

1. Clone the repository
2. Install myhtml and curlpp
3. Compile the program by running 'g++ steamgifts.cpp -Llib64 -lcurl -lcurlpp -lmyhtml' on a linux machine.
4. Create a file called 'cookies' in the same folder as the compiled c++ file.
5. Paste your PHPSESSID from steamgifts.com into the 'cookies' file.
6. Run the program with ./a.out

Example:
```
git clone https://github.com/runi95/beepboop
g++ steamgifts.cpp -Llib64 -lcurl -lcurlpp -lmyhtml
echo 'ffffffffff0f0fffff00fffff00ff0000f0fff0ffffff0ffff00fff00fff0ff0f'>cookies
./a.out
```

## License

This project is under curlpp and myhtml's license which can both be found in their git repositories which have been linked to at the start of this file.

Other than that you're free to do whatever you want with this software, however I can not promise this will work as intended after updates from steamgifts nor can I guarantee that users of this software won't get banned for breaking any rules steamgifts has put up so use at your own risk!
