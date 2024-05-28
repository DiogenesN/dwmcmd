# DWMCmd
A small command line utility for Wayland that shows a list of currently opened windows and activates by title or app_id.
It was tested on Debian 12 on Wayfire.

# What you can do with DWMCmd
   1. List all currently opened windows titles and app_ids.
   2. Activating/raising up the window by providing the title or app_id.

# Before building
   install the following libs:

		make
		pkgconf
		libwayland-dev

   on Debian run the following command:

		sudo apt install libwayland-dev make

# Installation/Usage
  1. Open a terminal and run:

		chmod +x ./configure
		./configure

  2. if all went well then run:

		make
		sudo make install
		 
		(if you just want to test it then run: make run)

  3. first list all the titles and app_ids:
  
		dwmcmd --list-all

  4. pick any title or app_id from the list and run:

		dwmcmd title: Google Translate - Brave
		dwmcmd app_id: brave-browser

# Hint
	Some titles need to be put in double quotes e.g.:

		dwmcmd title: "Google Translate - Brave"

That's it!

# Support

   My Libera IRC support channel: #linuxfriends

   Matrix: https://matrix.to/#/#linuxfriends2:matrix.org

   Email: nicolas.dio@protonmail.com
