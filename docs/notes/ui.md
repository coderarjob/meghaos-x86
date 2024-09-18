# Megha Operating System V2 - General
------------------------------------------------------------------------------

## User events in GUI environment
categories: note, independent
_18 Sept 2024_

Operating system communicates with user processes using Events. Every process has a events queue
where items corresponding to events will be put by the operating system for processes to process.

Some example of events are keypress/release events, mouse movement/click, paint, timer ticks or
yield request. Most (if not all) of these events will be sent to only the 'Active' process, since
'In-Active' processes are just idle.

### Example of handling events in GUI process

GUI controls like button, textbox are simple graphics widgets, they do not get the events directly.
The event loop in the process determines which control the events should go to. For example
keypress/release events should go to the active control. That control can handle the event or just
does nothing with it.


```c
Control button1, textbox1;
void ui_setup()
{
    // control::keyup etc are function pointers which
    // are called from event_loop when corresponding events come up.
    
    button1.keyup = s_button_keyup;
    button1.keydown = s_button_keydown;
    button1.paint = s_button_paint;
    
    textbox1.keyup = s_textbox_keyup;
    textbox1.keydown = s_textbox_keydown;
    textbox1.paint = s_textbox_paint;
}

void event_loop() 
{
    switch(get_event()) {
        case EVENT_KEYUP:
            e = get_event_data();
            if (button1.isactive() && button1.keyup(e)) {
                continue;
            }
            if (textbox1.isactive() && textbox1.keyup(e)) {
                continue;
            }
            break;
        case EVENT_KEYDOWN:
            e = get_event_data();
            if (button1.isactive() && button1.keydown(e)) {
                continue;
            }
            if (textbox1.isactive() && textbox1.keydown(e)) {
                continue;
            }
            break;
        case EVENT_PAINT:
            button1.paint();
            textbox1.paint();
            break;
        case EVENT_YIELD_REQ:
            sys_call_yield();
            break;
        case EVENT_EXIT_REQ:
            // Cross button or some was clicked. OS asking the process to be closed.
            break;
    }
}

int main()
{
    ui_setup();

    while(1) {
        event_loop();
    }
    return 0;

}
```

At the level of the above example the `Controls` do not store their states/properties. That can
either be done though a wrapper or will remain with the process itself. What I mean is for example
the `Checkbox` control does not keep its checked state. When painting it just draws differently
depending on the state. Both painting and state keeping is done by code in the process not in the
control. The `Control`s thus just represents some property visually, but they do not own them.

Inside a long running loop, we can check for events using the same `get_event()` function and take
some action if required. The below example shows how the YIELD event is handled within the long
running loop in a function.

```c
function long_running_func()
{
    // ...
    while(!is_done) {
        // ...
        if (get_event() == EVENT_YIELD_REQ) {
            sys_call_yield()
        }
        // ...

    }
    // ...

}
```

### Cooperative multitasking using Timer

For Cooperative multitasking to work processes must yield regularly.
* When there is nothing for it to do.
* Or periodically in the middle of a long running task.

The OS can send EVENT_YIELD_REQ at regular intervals a cue to the process that is been running for a
long time and its time to give up control.

This mechanism can also be used to detect 'bad' processes and some action can be taken for them. A
process which is not responding to YIELD events can be considered 'rogue' or 'bad.

### Points to consider?
1. How to handle multiple windows of a single process? Should every window have an event queue? A
   process always have a single window so always have at least one event queue. Or there need to be
   two types of events - GUI events which are handled by each window and process events which are
   handled by processes.
2. Is there a need for EVENT_PAINT event? With compositing window manager it will paint from the
   window's private frame buffer. It can repaint as many times, but frame at the time of frame swap
   will be displayed. Should it paint multiple times?

------------------------------------------------------------------------------

## Rethinking the idea of Files
categories: note, independent, feature
_8th Nov 2023_

Think of a magical world, where document's context changes automatically based on edits made in
another document. In this world there data is not constrained to a specific file.

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

* Though I like wot work on the command line, I find that it can be made more useful. These terminal
  emulators are graphical applications without graphics. In the end, I do no want to imitate
  teletype in MeghaOS - makes no sense for me copy UNIX ideas.
