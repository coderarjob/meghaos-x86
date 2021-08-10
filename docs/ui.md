## Megha Operating System V2 - General
## User Interface notes
------------------------------------------------------------------------------
_4th Dec 2020_

Here are the **goals**. 
* Applications comprise of graphical or functional components. These components
  can be called and used from any other application installed in the system. 
  1. For example: The 'emboss filter' from Photoshop can be used on a clipart 
     in Word. Also spelling correction features from Word can be used in 
     Photoshop.
  2. The 'Calculate contact angle' feature from ACAM application can be used in
     an image taken from Camera or an image created in Paint. The said feature
     can be called from any application, without separately opening the ACAM
     application.
  The walls of application should disappear and the features in it can be used
  seamlessly across the whole operating system.
  **Applications are add-ons to the Operating System features.**

* The **Editor** program will serve an important place in the UI. It is where
  the user can type commands and also write programs or paint. As a matter of
  fact, any document can have embedded commands that can be executed from
  inside the editor. (Inspiration from Oberon Operating System, and Plan9 rio)

* Graphical components can also be borrowed from another application. You like
  the progress bar implementation of a game. You can use it in your application
  as well. And the game can itself be able to use components that came with
  other applications.

* Tilled window manager. With controls to _Create_, _Move_, and _Resize_
  windows.

* Keyboard shortcuts can play a large role. These shortcuts can be easily
  created for any of the features in the operating system.

* Computer opens to an editor with the options last saved (commands in the
  toolbar). The user can use the editor to execute commands or write document
  or programs. Any call to a feature will open another window and keep the
  window associated to that command (along with the arguments) for future
  displaying of output. That is, when the same command is executed with the
  same argument, it will be displayed in the same window.

