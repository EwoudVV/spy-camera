# Spy Camera Project Journal

**Total Time: 27 hours**

Hour 0-2: planning

this started as a history project, for the cold war, in which i needed to make something related to it. 
i really like electronics, so i had the idea to make a spying device. my dad had the idea to do a clock, and i already had an esp-32 cam.

made a list of the parts I would need:

esp-32 cam (already had)
3.3v buck converter (already had)
9V battery (already had)
a clock

Hour 2-4: finding the clock

i went to a thrift store with my dad to look for something that looked old enough for the cold war

Found a digital alarm clock that seemed good. i bought it and took it home.

i opened it to see how everything was laid out before deciding where the camera could fit.

Hour 4-9: Disassembly and Camera Mount

i removed the front cover (it was annoying, all the screws were stripped and there were clips) and measured where the camera lens needed to be.

marked the location and drilled a hole through the plastic front panel.

mounted the esp-32 cam behind the front panel and checked that the camera had a clear view.

Hour 9-12: power system

at first, i tried using a small single cell lipo, but the esp wasn't getting enough power, because it kept shutting off. so i decided to use a 9v battery.

i installed a buck converter to step the 9V battery down to the esp-32 cams operating voltage, 3.3v.

i made sure the original clock electronics still worked when plugged into the wall while the camera remained battery powered.

Hour 12-20: programming

i started writing and modifying the esp-32 cam code.

i configured it to:

create its own wifi hotspot (like the phones)
stream live video
automatically display a captive portal when someone connects, so i dont have to manually type the ip address

spent quite a while debugging wifi settings and making sure the video stream stayed stable. in the end, the esp32 cam was just broken. 
i was annoyed because i spent a lot of time trying to get it to work. it wasnt even showing up in the device list anymore. but luckily i had another esp-32 cam to use.

eventually got the camera streaming reliably.

Hour 20-23: assembly

installed everything permanently inside the clock. i didnt find any replacement screws, so i ended up hot gluing the shell of the clock to the chassis.

mounted the battery and buck converter so they wouldn't move around inside.

The outside still looked like an ordinary alarm clock except for the camera opening.

Hour 23-24: Testing

tested different stuff:

Clock plugged into wall
Clock unplugged
Battery powering only the camera
Connecting multiple devices
Captive portal opening correctly
Live video quality

during testing, i found a weird issue, where sometimes the captive portal would pop up when i connected it to the wifi, and sometimes it wouldnt. 
i ended up finding out that only one device can use the captive at a time, even though the wifi still showed up.

Hour 24-25: Documentation

created the GitHub repo

wrote the readme

wrote the journal (yes this)

uploaded the arduino sketch and finalized the repository.

reflection

this project took about 25 hours from start to finish. 
the hardest parts were getting the esp-32 cam software working reliably with the captive portal. 
in the end, I achieved my goal of creating a hidden camera disguised as a functioning 1980s alarm clock while keeping the original clock operational.
