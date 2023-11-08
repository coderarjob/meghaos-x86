# Megha Operating System V2 - General
------------------------------------------------------------------------------
## Rethinking the idea of Files
categories: note, independent, feature
_8th Nov 2023_

Think of a magical world, where document's context changes automatically based on edits make in
another document. In this world there data is not constraints of a specific file.

When I started thinking on these lines, it was clear that such a system will provide several
advantages, such as eliminating the need to manually update dependent documents.

In this data-centric approach, traditional concept of files becomes just one way to view the data.
Think of an endless desk or notebook, divided into distinct 'areas'. When you start working, you
create a new area (akin to clearing your desk).

Take the example of a simple C project. You create an area and simply start writing your program and
documentation together, without the concept of traditional files. Specific "views" can then be
created to show specific sections of this area. These "views" are similar to traditional files.
For instance a 'Readme' view may show the documentation, usage and example from the area, while a
"main.c" view would show the C functions and interface documentation.

Furthermore, views can have a formatting & layout information for its "data". This means Rich format
documents is also possible.

The concept of "area" and "views" serves as the base for sharing data at a higher level, but will
sharing data work within a single "area" without duplication. For instance, if a financial or phone
number appears in multiple locations with this "area". One approach can be to attach metadata (such
as a name) to this number the first time it's used, and then refer to that metadata in other
instances. This way duplicates are removed. Keeping only the first occurrence writable we can also
have some control who gets to change this number.

### Doubts and questions:
1. When a content of file is linked to content in another file, how to have backups or see the
   previous version?
2. For simple text documents, a modified text editor will be able to compile and display without a
   problem. But what happens to a computer language compiler?
3. How to export/import data to and from conventional file systems?
   Well export may mean to flatten the different views into separate files. Import may work the
   opposite way - there will be one area per file and one view showing the complete file.

------------------------------------------------------------------------------

## User Interface notes
categories: note, independent, feature
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

