# Tutorial-FreeRTOS-Setup

In this tutorial, I will show how to set up a FreeRTOS project for the Tiva C TM4C123x device in Keil uVision.  In Part 1, a FreeRTOS project will be created using the CMSIS libraries and Keil software packs; and in Part 2, I will show how to incorporate FreeRTOS directly into an empty uVision project and configure it manually.  

The method used in Part 1 requires fewer steps and is the quicker way to get a project up and running.  However, if you prefer a minimalist approach and/or a non-CMSIS implementation, then you might want to use the second method (Part 2) instead.  I recommend going through the steps in Part 1 before trying Part 2, because (with a few modifications) you will be able to reuse the FreeRTOSConfig.h file that that is generated there.

[![](http://img.youtube.com/vi/2OqZpkLucUY/2.jpg)](https://youtu.be/2OqZpkLucUY)<br>
[Watch the video - Part 1](https://youtu.be/2OqZpkLucUY)

[![](http://img.youtube.com/vi/lj56g2fwfrQ/default.jpg)](https://youtu.be/lj56g2fwfrQ)<br>
[Watch the video - Part 2](https://youtu.be/lj56g2fwfrQ)