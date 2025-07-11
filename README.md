# slidenote
Experimental repository for making a simple note taking app that allows for the usage of hiding away with the usage of a hot key. Inspired by Pierre Châtel's sidenote.

Uses [Qt6](https://www.qt.io), and [QHotkey](https://github.com/Hellmark/QHotkey), set up to be built using [CMake](https://www.cmake.org)

## MacOS notes

if you are wanting to run the version that is being built via Github actions, currently Apple security might prevent you. After extracting from the DMG, run the following command upon it:

```
sudo xattr -rd com.apple.quarantine Slidenote.app
```

This will bypass some of the security and allow it to run normally.

It is planned to have an Apple Developer ID certificate added in the near future, but until that can be done this is an appropriate work around.

Since everything is done using github actions, you can easily compare against the code for that particular release if you have questions about security in the meanwhile.
