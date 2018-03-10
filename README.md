# Arduino-Weaponry

**Here are my very first projects!**

## Automatic-Reloading-Crossbow (ARC)
The Automatic Reloading Crossbow is a final project from my introductory electrical engineering class (ECE110), built and programmed with the help of a partner (Aniruddha Pispati), but the design is very original! It's built out of cardboard, paper, and chopsticks, and does exactly what its name suggests- it fires little cardboard darts. With the help of an ultrasonic range sensor in the front, this crossbow has the potential to automatically sense and fire upon any movement it detects- however, both its firing and detection range are quite low- about 3 ft. (and that was years ago, and now the crossbow is deteriorating in storage). It's reloading mechanism is quite clever- a cardboard cartridge held above the firing tube gravity-feeds darts one by one with the help of a servo. However, the full motion of pulling back the arms, dropping a dart, and firing, takes upwards of 2 seconds per shot. Overall, a very fun project to look at, but it falls short of any reasonable functions you might expect of even a cardboard crossbow.


## Lego Nerf Gun
The Lego Nerf Gun is my second project, where my goal was to improve on the crossbow's horrendous range and terrible reload times. It's designed and built entirely by me! For fun, over the summer. Also, it's compatible with Nerf magazines!The structure and mechanical portions are built primarily out of LEGO (and a PVC pipe), but make no mistake, this ain't just your regular LEGO model... It's powered by 8 AA batteries, with motors and electrical components enabling it to be a fully-functional, and quite powerful Nerf gun. It works by spinning up two flywheels when a button is depressed, and then pushing darts one-by-one into the flywheels when a trigger is pulled. The pusher mechanism took a bit to design, but it works quite well, and uses a microswitch to reset to the same position after each shot, or to count how many shots it has taken at a time, for things like burst-firing. But on top of being able to shoot darts up to 110 feet per second, at a rate of ~3 darts per second..

#### It's filled with lots of other fun features!

* Can detect and calculate the speed of each dart which exits its barrel (with the help of two photogates).

* Detects when a magazine is currently inserted (and comes with a nice blue/red LED).

* Built-in ammunition tracker, which resets each time a magazine is reloaded, and decrements with each dart fired. You can tweak the number of darts you have with a rotary encoder, as well as pressing the encoder to set your magazine size.

* Comes with a voltage meter, to tell you when your batteries are low!

* Features 3 different firing modes! Semi-automatic, burst-fire (3 shots) and full automatic (for when you want maximum fun!) You can control these firing modes by pressing a very pretty glowy button on the side.

* Has an array of cool red, green, and blue LEDs at the top, which light-up in order when you rev the gun, giving you an estimate of how much power a dart fired at that moment will have. Also just really pretty.

* Advanced anti-bug and debouncing mechanisms, to counteract the electrical noise of the motors and problems with the components. As a last-ditch measure, it has a watchdog to restart if the controller stops responding. This part took a lot of time.

* And last but not least, has an on-off switch that lights up! 'Cause that's important too.


## Final Words
There are currently no guides or any pictures available online, but that may change sometime!
I'm also planning on building a successor Nerf gun, which will be a lot more durable and not built out of LEGOs.
More on that later, maybe.

Thanks for coming by! Feel free to browse the code or do whatever you'd like with it.
