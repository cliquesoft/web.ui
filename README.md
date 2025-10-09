# [PREAMBLE]

	Thanks for taking interest in web.ui! This project provides a
	minimalistic graphical software interface using webpages. Similar to
	Mozilla's (now deprecated) Chromeless project (formerly Prism), this
	software enables an author to create web-based applications and
	services that look and fell like typical compiled software.

		TODO
		- limit I/O to loopback interface and optionally LAN
		- incorporate other security features
		- compile using shared libs to reduce footprint




# [FOR THE IMPATIENT]

	To Compile:
		1. cd /path/to/web.ui
		2. ./make [PREFIX=/packaging/dir]

	To Compile and Install:
		1. cd /path/to/web.ui
		2. ./make [PREFIX=/packaging/dir] install

	Examples of Starting:
		# Start using an 800x600 pixel window
		web.ui -g 800x600

		# Start using an 800x600 window at 100,50 coordinates
		web.ui -g 800x600+100+50

		# Start with the app saving its data in a home directory
		web.ui -s "~/.web.ui/app_name"

		# Override the HTML <title> value for the window
		web.ui -t "Use this instead"




# [DESIGN GOALS]

	Unlike Mozilla's project, this one has some differences in its goals.
	Both share the concept of using web languages to generate an "app",
	however their project appeared to allow interaction with local and
	remote (Internet) sources. Essentially they would allow transforming
	any accessible "software" into what appeared to be an application to
	the end user. For security, our goals are to restrict interaction and
	permit ONLY communication with local sources - specifically the local
	loopback interface (127.0.0.1), and optionally LAN (192.168.x.x and
	10.x.x.x). Anything outside of those parameters would be considered
	WAN or Internet-based, which is much more suited for full-featured
	and secured web browsers. This should also help reduce the amount of
	security implementation necessary for this small project.

	Additionally, the Chromeless project wanted to allow for more access
	to the system than a normal browser would permit by exposing a new
	API specifically for that software. There are no intentions of adding
	or implementing anything of the sort in this project to maintain the
	exact same experience as using a normal web browser in this regard.
	Any and all elevated access to devices and data will have to be
	performed through the local web server.

	The last goal to mention here is the footprint and feature set. To
	prevent bloat and security issues, this software should be kept to
	an absolute minimum in order to allow a user to focus on the creation
	of the developer. To assist them when possible is certainly an effort
	to work towards, but adding in obscure features will most likely be
	rejected in the main git branch. Be mindful that this software will
	need to run in as many environments as possible, included lightweight
	embedded devices. Currently there is just a small set of features with
	this project, and absolutely no security measures have been added as
	outlined above. Suggestions and patches are obviously welcome from the
	community.




# [FUTURE DEV TIMELINE]

	Since we are working with several many projects (13 on github alone),
	we are going to provide an anticipated timeline of releases using
	internal staff. Obviously outside contribution will advance these
	forecasted dates.

	2025 Oct - completion of ModuleMaker for webWorks
	2025 Dec - migration of existing webWorks modules using ModuleMaker
	2026 Jan - migration of Tracker into webWorks and deprecation of
	           of standalone project
	2026 Feb - update paged to 2018 code base from ACME
	         - update pax to work with (TC) TinyCore Linux
	         - apply any patches for bug fixes to existing projects
	2026 Mar - update web.libs for dittodata and web.de
	2026 Jul - move code from web.de into cli.de and update the former
	           to use the latter via XML communication
	2026     - rest of 2026 tbd

