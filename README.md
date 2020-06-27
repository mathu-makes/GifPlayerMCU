# GifPlayerMCU
Gesture Controlled GIF Player [ATSAMD51 APDS-9960]

![GitHub Logo](/images/design-overview.jpg)

I wanted to take on a project that took an idea from concept to prototype. So far I have written most of the code, focusing on logic and userbility.
I used Adafruit's GIF player example as a starting point and began to build things out from there. As of writing I have a working program written in C++, a custom PCB design + schematic and a custom encloure design. I have just recently began work on a companion app written in Android.

Some of the core features I've built into the program so far are:

    Auto creation of folders at runtime if non present on device or SDcard using JSON
    Graphical user interface using LVGL library
    APDS-9960 gesture control + action button for menu naivgation & gif selection
    Ability to pause GIF playback
    Option to control GIF playback time
    Option to randomise GIF playorder
    Option to control TFT brightness
  
After building the core program, I then began to learn how to design a custom PCB. I made the decision early on to develop this project within the Adafruit ecosystem
as they produce very powerful MCUs. Not only that but they also create great learning material which so far has been incredibly helpful to me. As I have used the Adafruit 
ItsyBitsy M4 for prototyping, it was very easy to find Eagle PCB design and schematic resources. After gaining more confidence in Eagle and developing understanding of how the
ItsyBitsy functioned, I then began to design a custom PCB. Thereafter I then created a enclosure in Fusion 360. Again Adafruit learning resources helped here too, as I found
many YouTube tutorials that were invaluable.

My next steps are to put together a complete prototpye. I am looking into 3D printing services for the enclosure and PCB manufacturing houses for the PCB.
I have also created a basic bill of materials, using Digikey to source components. 

I have found this project my most rewarding so far as it has taught me many knew things about product design, electronics and programming.
