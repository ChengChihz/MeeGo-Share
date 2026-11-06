# MeeShare 1.005

This feature update adds file-opening actions and raises the incoming file-size limit.

- After receiving a single file, choose “Open now” if an installed app supports it; otherwise, or for multi-file transfers, open the receive folder to browse the files
- The per-file receive limit increases from 64 MiB to 200 MiB; the total per request increases from 128 MiB to 400 MiB, with up to 20 files
- Files continue to stream to disk; available storage is checked before receiving, with improved temporary-file cleanup and recovery after interrupted transfers

> N9 hardware transfer test: receiving a complete 200 MiB file took about 73.3 seconds; sending it back from the N9 took about 201.5 seconds. For reference only; speeds depend on the files and WiFi network conditions.

Files open in their associated system apps. MeeShare does not include a file viewer.
