Attempt to recreate my 'Image Editor' in C. Simple program for compositing two images together (idea being to facilitate process of watermarking images, for those with no image editing software skills). 
User can change logo x and y positions, alpha and scale; rotation is also possible for both logo and base images. 

Uses GTK4 library for C, without which this project would not have been possible.

First proper attempt at coding in C. In total, around 6 days spent between set-up and actual programming.

Some notes:
- 32-bit jpegs don't seem able to be loaded to pixbuf, neither to texture. Interestingly, it can be loaded directly in Picture widget on startup.
  - if previously converted to .png, said files give no problems and other non-jpeg 32-bit images seem to work fine.
  - our program in C# does not have this limitation. Investigate use of Gelg library? (https://discourse.gnome.org/t/rendering-16-bit-image-in-gtk/5615 , https://developer.gimp.org/api/gegl/index.html#structs)
  - in any case, should not pose much of a problem as in theory 32-bit jpegs should not be a thing (although Corel indicates CMYK 32-bit jpegs as a possibility. See https://product.corel.com/help/CorelDRAW/540240626/Main/EN/Doc/wwhelp/wwhimpl/common/html/wwhelp.htm?context=CorelDRAW_Help&file=CorelDRAW-JPEG-JPG.html) 
- currently no filters added to file selector, so users can select ANY type of file, although non-image files (obviously) will not be processed and return errors.
