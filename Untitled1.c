#include <gtk/gtk.h>


int iImage1_w;
int iImage1_h;
int iImage2_w;  // logo width
int iImage2_h;  // logo height
//int iW; // width of final image
//int iH; // height of final image

GtkWidget *window;
GtkFileDialog *diag;
GtkWidget *imgbox;


// buttons
GtkWidget *button;
GtkWidget *button2;
//GtkWidget *button3;
GtkWidget *button4;
GtkWidget *button_rl;
GtkWidget *button_rr;

// toggle buttons
GtkWidget *rotImg;
GtkWidget *rotLogo;

//scales
GtkWidget *scale_x;
GtkWidget *scale_y;
GtkWidget *scale_s;
GtkWidget *scale_alpha;


// path labels
GtkWidget *path01;
GtkWidget *path02;

// actual paths
gchar *cImgPath = "EMPTY!";
gchar *cLogoPath;
gchar *sImgPath_short;

// actual files
GFile *fileImg;
GFile *fileLogo;
gchar *sImgName;

// files loaded to buffer
GdkPixbuf *pbImg;
GdkPixbuf *pbLogo;

// texture projected to previewer widget
GdkTexture *myTex;


// bool used to determine if BOTH file paths are filled
// called by 'enable_button()' function
bool bPaths_filled (){
    if (!cImgPath|| cImgPath == "EMPTY!" || !cLogoPath || cLogoPath == "EMPTY!"){

        return false;
    }

    return true;
}

static void enable_button(){

    // make sure to keep 'GENERATE' button disabled if selected image file not loaded to buffer (eg., 32-bit jpeg images)
    if (!pbImg || !pbLogo || !bPaths_filled()){
        g_print("\n [WARNING]: One of the images could not be loaded to buffer. \n If LOGO or MAIN IMAGE not yet chosen, ignore this. \n");
        gtk_widget_set_sensitive(button4, false);       // disable image composition button
        gtk_range_set_range(scale_alpha, 255.0, 255.0); // disable alpha scale
        return;
    }

    g_print("\n [INFO]: all paths filled and all images loaded to buffer. Image composition ENABLED. \n");
    gtk_widget_set_sensitive(button4, true);        // enable 'GENERATE' button if BOTH paths filled (and loaded into pixel buffer)
    gtk_range_set_range(scale_alpha, 0.0, 255.0);   // enable alpha scale

    // so that we can reset previewer image if we set IMAGE or LOGO to empty (e.g., opening file picker but cancelling operation)
    // given both 'open_diag_image()' and 'open_diag_logo()' func. call this function, calling 'load_images()' here avoids code duplication...
    generate_image(NULL, false);
}





void choose_image(GtkFileDialog* self, GAsyncResult *res){
    fileImg = gtk_file_dialog_open_finish (self, res, NULL);

    if (fileImg == NULL){
        g_print("[ERROR]: No image chosen!\n");
        cImgPath= "EMPTY!";
        sImgName= NULL;
        gtk_label_set_label(path01, cImgPath);
        gtk_widget_set_sensitive(button4, false); // keep 'GENERATE' button disabled if EITHER paths NULL

        // would be ideal to disable scales. Problem is, when 'disabling' (setting min and max to same value), scales also reset, & user might want to keep scale at same value between selections...

        // we also need to erase our PICTURE widget, else if user cancels file selection, previous image continues rendered on previewer window
        gtk_picture_set_filename(imgbox, "");   // let's send an "empty" picture to widget
        return;
    }

    cImgPath = g_file_get_path(fileImg);    // store file's path to string -> *not 'string' but 'G' string... API can be misleading, as function ('g_file_get_path') returns a CHARACTER, not a STRING! (https://stackoverflow.com/questions/75379675/gio-resolve-path-with-symlinks)

    if (cImgPath == NULL){
        sImgName = NULL;                                                                                          // just to make sure property is reset
        g_print("[ERROR]: Img file chosen, but PATH still empty! \n");
    }else{

        g_print("[INFO]: Success! Selected IMAGE file loaded: \nPath: %s", cImgPath);
    }


    gtk_label_set_label(path01, cImgPath);  // get our label to show selected image path (or else, display "empty" message);


    // image analysis must be done here (getting w & h values), so our scale widgets can be processed BEFORE hitting 'Generate' button
    // (remember, if w or h of logo greater than that of main image, we disable respective sliders);
    pbImg = gdk_pixbuf_new_from_file (cImgPath, NULL);  // get our chosen image to buffer

    // find out and store MAIN IMAGE dimensions:
    if (pbImg){     // 32bit jpeg images not processed properly when added to pixbuf (in theory, jpg should not support 32-bit images, although Corel indicates CMYK 32-bit jpegs as a possibility. See https://product.corel.com/help/CorelDRAW/540240626/Main/EN/Doc/wwhelp/wwhimpl/common/html/wwhelp.htm?context=CorelDRAW_Help&file=CorelDRAW-JPEG-JPG.html)
        iImage1_w= gdk_pixbuf_get_width(pbImg);
        iImage1_h= gdk_pixbuf_get_height(pbImg);

        printf("\n[DEBUG]: Main image is %ux%u pixels. \n", iImage1_w, iImage1_h);

        // for convenience, we can also display our chosen MAIN IMAGE on preview (user can then easily check if main image the correct one, even before choosing Logo)
        myTex = gdk_texture_new_for_pixbuf(pbImg);  // fill our texture with our MAIN IMAGE pixel buffer data,
        gtk_picture_set_paintable(imgbox, myTex);   // and then fill our image preview widget with our texture;
    }else{
        printf("\n [ERROR]: Main image could not be loaded to or found in buffer, returning empty! \n [WARNING]: Check if not using 32-bit .jpeg images, which are not supported... \n");

		// we need to make sure to clear the Previewer box here, given any previously selected images will still be rendered (just for sake of looking neat... no major impact in functionality)
		gtk_picture_set_filename(imgbox, "");   // let's send an "empty" picture to widget

		// maybe clear the path label, too? Or give different message?

        cImgPath= "EMPTY!";						// let's make our PATH variable empty (so other functions that depend on it don't get confused thinking the file is valid)
        gtk_label_set_label(path01, "Selected file not supported!");	// inform user of situation
    }

    //toggle_scale_x();
    //toggle_scale_y();

    enable_button();
}


static void choose_logo(GtkFileDialog* self, GAsyncResult *res){

    fileLogo = gtk_file_dialog_open_finish (self, res, NULL);

    if (fileLogo == NULL){
        g_print("[ERROR]: No logo chosen!\n");
        cLogoPath= "EMPTY!";
        gtk_label_set_label(path02, cLogoPath);
        gtk_widget_set_sensitive(button4, false); // keep 'GENERATE' button disabled if EITHER paths NULL

        // would be ideal to disable scales. Problem is, when disabling, scales also reset, & user might want to keep scale at same value...

        // we also need to erase our PICTURE widget, else if user cancels file selection, previous image continues rendered on previewer window
        gtk_picture_set_filename(imgbox, "");
        return;
    }


    g_print("\n [INFO]: Success! Selected LOGO file loaded.\n");

    cLogoPath = g_file_get_path(fileLogo);    // store file's path to string -> *not 'string' but 'G' string... API can be misleading, as function ('g_file_get_path') returns a CHARACTER, not a STRING! (https://stackoverflow.com/questions/75379675/gio-resolve-path-with-symlinks)

    if (cLogoPath != NULL){
        g_print(cLogoPath);
    }else{
        g_print("\n [ERROR]: Logo file chosen, but PATH still empty! \n");
    }

    gtk_label_set_label(path02, cLogoPath);


    // we need to do image analysis here, so our scale widgets can be processed BEFORE hitting 'Generate' button
    // find out and store LOGO dimensions:
    pbLogo = gdk_pixbuf_new_from_file (cLogoPath, NULL);

    if (pbLogo){
        iImage2_w= gdk_pixbuf_get_width(pbLogo);
        iImage2_h= gdk_pixbuf_get_height(pbLogo);
        printf("[DEBUG]: Logo is %ux%u pixels. \n", iImage2_w, iImage2_h);
    }else{
        printf("\n [ERROR]: Logo could not be loaded to or found in buffer, returning empty! \n [WARNING]: Check if not using 32-bit .jpeg images, which are not supported...\n");
        return;
    }

    //toggle_scale_x();
    //toggle_scale_y();

    enable_button();
}

static void choose_loc (){

}

// send function name through gpointer, merge both functions below into one?
static void open_diag_image (GtkWidget *widget, gpointer   data){
    g_print("BUTTON 01 clicked. Choosing image...\n");

    gtk_file_dialog_open (
      diag,
      window,
      NULL,
      choose_image,
      NULL
    );
}

static void open_diag_logo(GtkWidget *widget, gpointer   data){
    g_print("BUTTON 02 clicked. Choosing logo...\n");

    gtk_file_dialog_open (
      diag,
      window,
      NULL,
      choose_logo,
      NULL
    );
}

static void open_diag_save(GtkWidget *widget, gpointer   data){
    g_print("BUTTON 03 clicked. Choosing save loc...\n");

    gtk_file_dialog_open (
      diag,
      window,
      NULL,
      choose_loc,
      NULL
    );
}

// determines how much we can move logo along axes
double calc_offset(int h1, int h2){

	double fMaxOffset= (double) h1 - h2;
	//gtk_range_set_range(scale, 0.0, fMaxOffset);
    if (fMaxOffset < 0.0)
        return 0.0;
	return fMaxOffset;
}

// messing with strings is just painful in C. Code courtesy of 'David Heffernan': (https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c#:~:text=Use%20strcat%20to%20concatenate%20two%20strings.%20You%20could%20use%20the%20following%20function%20to%20do%20it%3A)
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


int iFinalLogo_h;
int iFinalLogo_w;

bool bIsRunning = false;    // key property, used to make sure 'generate_image()' func. does not run multiple instances in parallel
// ~~~~~~~~~~~~ START: image generation code
void generate_image(GtkWidget *widget, gpointer   data){

    g_print("\n *[GENERATE_IMAGE] Image generation solicited.* \n");
    // alpha slider calls this function right on app start (we set the slider to MAX (255.0) on app start, triggering the call),
    // naturally throwing errors in console.
    // this check should stop function running on start...

    // trying to call this function while Main Image or Logo buffers are empty also generate errors
    // (e.g., loading 32-bit .jpeg: image is loaded and path filled, but pixbufs don't accept the file, so checking for path only not enough)
    if (!bPaths_filled() || !pbImg || !pbLogo){
        g_print("[DEBUG]: Image generation called, but either image paths, base image or logo pixel buffers returned empty.\n Aborting call... \n");
        return;
    }


    // gatekeeping code, stops function from running multiple instances in parallel
    if (bIsRunning){
        g_print("\n [ERROR]: Process already running. WAIT! \n ");
        return;
    }
    g_print("--> --> [GENERATE_IMAGE][START] Image generation STARTED. <-- <--\n");
    bIsRunning= true; g_print("--> [GENERATE_IMAGE] Function locked. <--\n ");

    bool canSave = (bool) data; // passed when user clicks 3rd button ('GENERATE') or changes scales

    // copy the buffers so we can keep originals intact if needed later
    GdkPixbuf *gpbImg = gdk_pixbuf_copy (pbImg);   // our canvas, where gbpLogo will be overlayed on top
    GdkPixbuf *gpbLogo = gdk_pixbuf_copy (pbLogo);
    iFinalLogo_h= iImage2_h;
    iFinalLogo_w= iImage2_w;

    double iScaleFactor_1 = 1.0;
    double iScaleFactor_2 = 1.0;

    // differently from Cairo, 'gdk_pixbuf_composite()' function does not allow for our plotted image to be bigger than source
    // so we need to make sure logo is scaled appropriately or else risk it being cropped...

	// find ratio, so we can preserve aspect ratio
	// keep shrinking both axes until logo completely fits
    if (iImage2_w > iImage1_w && iImage2_h > iImage1_h){
		printf("[INFO]: Both Logo height and width greater than main image and will be scaled to fit...\n");


		//1st, equalise y-axis (height);
		iFinalLogo_h= iImage1_h;

		// and conform width to aspect ratio:
		float ratio= (float) iImage2_h / iImage2_w;    // determines h-to-w ratio;
		iFinalLogo_w= iFinalLogo_h / ratio;

		// thing is, this does not mean final logo will fit in base image with these new dimensions;
		// so we double-check:

		if(iFinalLogo_w > iImage1_w){
			// still too large
			printf("[DEBUG]: width still too large. 2nd round of scaling needed... \n");

			iFinalLogo_w = iImage1_w;	// if width still not to size, no choice but to force it to base img dimension...

			// and update height as needed, to keep aspect ratio uniform;
			iFinalLogo_h= iFinalLogo_w * ratio;


		}

		// do the magic (see below; we use this instead of calling 'gdk_pixbuf_scale_simple()')
        iScaleFactor_1 = (double) iFinalLogo_w / iImage2_w;
        iScaleFactor_2 = (double) iFinalLogo_h / iImage2_h;

		printf("[DEBUG]: OG. WIDTH: %dpx; NEW WIDTH: %dpx\n", iImage2_w, iFinalLogo_w);
		printf("[DEBUG]: OG. HEIGHT: %dpx; NEW HEIGHT: %dpx\n", iImage2_h, iFinalLogo_h);

    }else if(iImage2_h > iImage1_h){
        printf("[INFO]: Logo height greater than main image and will be scaled to fit...\n");

        iFinalLogo_h = iImage1_h;                       // if logo height greater than base img height, equalise them;

        // for keeping aspect ratio constant, we need to calculate h-to-w ratio and new width:
        float ratio= (float) iImage2_h / iImage2_w;    // determines h-to-w ratio;
        iFinalLogo_w= iFinalLogo_h/ratio;



        // determine scale factor. equation below extracted from source of function 'gdk_pixbuf_scale_simple()' (https://gitlab.gnome.org/GNOME/gdk-pixbuf/-/blob/master/gdk-pixbuf/gdk-pixbuf-scale.c)
        //(double) dest_width / src->width
        //(double) dest_height / src->height
        // .: this was necessary as firing mentioned function in sequence caused huge spikes in memory use! Function itself simply a wrapper for calling 'gdk_pixbuf_composite()',
        // so we only had to apply same equation it used to determine scale factor and insert it into OUR call of 'gdk_pixbuf_composite()' below...
        iScaleFactor_1 = (double) iFinalLogo_w / iImage2_w;
        iScaleFactor_2 = (double) iImage1_h / iImage2_h;


        gtk_range_set_range(scale_y, 0.0, 0.0);         // disable y-axis scale
        printf("OG. WIDTH: %dpx; NEW WIDTH: %dpx; RATIO: %f \n", iImage2_w, iFinalLogo_w, ratio);

    }else if (iImage2_w > iImage1_w){
        printf("[INFO]: Logo width greater than main image and will to be scaled to fit...\n");

        iFinalLogo_w = iImage1_w;

        float ratio= (float) iImage2_h / iImage2_w;    // determines h-to-w ratio;
        iFinalLogo_h= iFinalLogo_w*ratio;


        //gdk_pixbuf_scale_simple (pbLogo, iImage1_w, iFinalLogo_h, GDK_INTERP_BILINEAR);
        iScaleFactor_1 = (double) iImage1_w / iImage2_w;
        iScaleFactor_2 = (double) iFinalLogo_h / iImage2_h;


        gtk_range_set_range(scale_x, 0.0, 0.0);         // disable x-axis scale
        printf("OG. HEIGHT: %dpx; NEW HEIGHT: %dpx; RATIO: %f \n", iImage2_h, iFinalLogo_h, ratio);
    }



    // we need to clamp fUserScale's max value! Or else, scaling may extrapolate base image dimensions!
    double iMaxScaleFactor_x = (double) iImage1_w / iFinalLogo_w;  // by how much can Logo's x-axis expand without going over Base Img's?
    double iMaxScaleFactor_y = (double) iImage1_h / iFinalLogo_h;  // same, but for height/y-axis

    // the smallest value is our limiter! if x-axis can grow by 2.6 times, but y-axis can only do so 1.5 before hitting axis limits, whole image can only grow by 1.5 if we want to maintain aspect ratio!
    if (iMaxScaleFactor_x > iMaxScaleFactor_y){
        gtk_range_set_range(scale_s, 0.5, iMaxScaleFactor_y);
    }else{
        gtk_range_set_range(scale_s, 0.5, iMaxScaleFactor_x);
    }

    double fUserScale = gtk_range_get_value(scale_s);

    // re-calculate our final Logo size, taking in consideration desired user scale
    iFinalLogo_w= iFinalLogo_w * fUserScale;
    iFinalLogo_h= iFinalLogo_h * fUserScale;


    // unlock our x/y-axis sliders
    gtk_range_set_range(scale_x, 0.0, 1.0);
    gtk_range_set_range(scale_y, 0.0, 1.0);


    // calculate offsets based on user input (if disabled/locked, will return 0) and feed it to 'gdk_pixbuf_composite()' func.
    double fMaxOffset_x = calc_offset(iImage1_w, iFinalLogo_w); g_print("[DEBUG]: MAX possible X Offset: %fpx \n", fMaxOffset_x);
    double fMaxOffset_y = calc_offset(iImage1_h, iFinalLogo_h); g_print("[DEBUG]: MAX possible Y Offset: %fpx \n", fMaxOffset_y);

    int iUserOffsetX = (gtk_range_get_value (scale_x) * fMaxOffset_x); g_print("[INFO]: CURRENT X offset: %d \n", iUserOffsetX);
    int iUserOffsetY = (gtk_range_get_value (scale_y) * fMaxOffset_y); g_print("[INFO]: CURRENT Y offset: %d \n", iUserOffsetY);
    int iUserOffsetAlpha = gtk_range_get_value (scale_alpha);          g_print("[INFO]: CURRENT ALPHA offset: %d \n", iUserOffsetAlpha);



    int iQuality;        // determines the Interpolation Mode used by 'gdk_pixbuf_composite()' func.
    if (canSave == true){
        iQuality= 2;    // should either be 2 or 1 (balanced and highest quality, respectively);
    }else{
        iQuality= 0;    // cheapest mode, used for previewing output...
    }

    double fFinalScaleFactor_1 = iScaleFactor_1 * fUserScale;
    double fFinalScaleFactor_2 = iScaleFactor_2 * fUserScale;



    // our rotation/scaling code introduced some unforeseen issues; namely,
    // when rotating BOTH Logo & Base Img, we do some slider value swapping
    // (to make sure Logo's relative pos. is maintained constant with each rotation!)
    // problem is that, if we swap x-offset for y-offset instead of zeroing it,
    // it'll add up to our scaling and invalidate the final image due to out-of-bounds dimension (even though actual Logo img still within bounds)
    if (iUserOffsetX + iFinalLogo_w > iImage1_w){
        g_print("-> CRAP! Current offset + logo width extrapolates max bounds (x)! \n");
        while (iUserOffsetX + iFinalLogo_w > iImage1_w){

            iUserOffsetX -= 1;
        }
        gtk_range_set_value(scale_x, iUserOffsetX);

    }

    if (iUserOffsetY + iFinalLogo_h > iImage1_h){
        g_print("-> CRAP! Current offset + logo height extrapolates max bounds (y)! \n"); //iUserOffsetY = iUserOffsetY/fUserScale;
        while (iUserOffsetY + iFinalLogo_h > iImage1_h){

            iUserOffsetY -= 1;
        }
        gtk_range_set_value(scale_y, iUserOffsetY);
    }

    gdk_pixbuf_composite (gpbLogo, gpbImg, iUserOffsetX, iUserOffsetY, iFinalLogo_w, iFinalLogo_h, iUserOffsetX, iUserOffsetY, fFinalScaleFactor_1, fFinalScaleFactor_2, iQuality, iUserOffsetAlpha);  //GDK_INTERP_NEAREST (0) -> cheapest; GDK_INTERP_BILINEAR (2) -> balanced; GDK_INTERP_TILES (1) -> expensive;



    // code for saving file;
    // necessary to be conditioned, as saving and previewing share same mechanism
    //(to avoid unnecessary code duplication)
    if (canSave == true){
        g_print("[INFO]: SAVE mode active; \n");

        // FINAL NAME= FILE PATH + FILENAME + suffix ("_wmkd") + extension (".png")
        // F.N=  sImgPath_short + sImgName + "_wmkd" + ".png"
        GFileInfo *imgInfo = g_file_query_info (fileImg, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, 0, NULL, NULL);    // function for reading GFile attributes;
        gchar *sImgExtension = g_file_info_get_content_type (imgInfo);                                              // extract FILE TYPE (i.e., file extension), so we can remove it from filename later
        int iExtSize = strlen(sImgExtension);                                                                       // extension length

        sImgPath_short= g_file_get_path(fileImg);                                                                   // we canna do a copy ('sImgPath_short = cImgPath') as in C, its actually the pointer that gets carried over with strings (arrays); when changing 'sImgPath_short', 'cImgPath' gets changed as well (they both point to the same memory address (https://stackoverflow.com/questions/63742107/is-there-a-way-to-create-copies-of-variables-so-that-the-original-variables-don#:~:text=So%20plain_text%20doesn%27t%20store%20the%20string%20itself%2C%20it%20stores%20the%20address%20of%20the%20buffer%20containing%20the%20string.%20When%20you%20pass%20plain_text%20to%20substitute_strings%2C%20it%27s%20just%20receiving%20this%20pointer%20value%2C%20not%20a%20copy%20of%20the%20string.)
        sImgPath_short[strlen(sImgPath_short) - iExtSize] = 0;                                                      // removes file extension from file name... Credit to 'jhenninger' for the solution: (https://stackoverflow.com/questions/2736753/how-to-remove-extension-from-file-name#:~:text=Just%20replace%20the%20dot%20with%20%220%22.%20If%20you%20know%20that%20your%20extension%20is%20always%203%20characters%20long%20you%20can%20just%20do%3A)

        gchar *saveName= concat(sImgPath_short, "_wmkd.png");

        gdk_pixbuf_save(gpbImg, saveName, "png", NULL, NULL);   // save the result to file
        g_print("[INFO]: Output image saved: %s \n", saveName);

    }else{
        g_print("[INFO]: PREVIEW mode active; \n");

        // literally wasted a whole day just trying to get this shit to work,
        // given 'gtk_image_set_from_pixbuf()' func. deprecated in ver. 4.12...
        myTex = gdk_texture_new_for_pixbuf(gpbImg);
        gtk_picture_set_paintable(imgbox, myTex);

        // cleanup
        g_object_unref (myTex);
    }

    // cleanup
    g_object_unref(gpbImg);
    g_object_unref (gpbLogo);

    if (bIsRunning)
        bIsRunning= false; g_print("--> [GENERATE_IMAGE] Function unlocked. <--\n ");

    g_print("--> --> [GENERATE_IMAGE][END] Image generation FINISHED. <-- <--\n ");
}

bool bRotMain;
bool bRotLogo;
// pixbuf rotation function
// called from 'button_rl' & 'button_rr'
void rotate_image(GtkWidget *widget, gpointer   data){
    int i = (int) data;


    switch (i % 6){

        case 1:
            if (bRotMain){
                GdkPixbuf *pbTemp = gdk_pixbuf_rotate_simple (pbImg, 90);
                g_print("[INFO] Main image rotation requested (LEFT) \n");


                //////////   Clean-up Code      //////////////
                g_object_unref(pbImg);                  // clear pbImg from memory (already filled after choosing Base Image),
                pbImg= gdk_pixbuf_copy (pbTemp);        // copy our rotated, temporary pixel buffer to pbImg (now empty),
                g_object_unref(pbTemp);                 // clear our temp. pxbuf. NOTE: if we use 'direct link' (i.e., pbImg = pbTemp), pbImg also returns null after clearing pbTemp!
                /////////        END            //////////////

                //make sure to update dimension variables!
                iImage1_w= gdk_pixbuf_get_width(pbImg);
                iImage1_h= gdk_pixbuf_get_height(pbImg);
            }

            if (bRotLogo){
                GdkPixbuf *pbTemp2 = gdk_pixbuf_rotate_simple (pbLogo, 90);
                g_print("[INFO] Logo rotation requested (LEFT) \n");

                 //////////   Clean-up Code      //////////////
                g_object_unref(pbLogo);                  // clear pbImg from memory (already filled after choosing Base Image),
                pbLogo= gdk_pixbuf_copy (pbTemp2);        // copy our rotated, temporary pixel buffer to pbImg (now empty),
                g_object_unref(pbTemp2);                 // clear our temp. pxbuf. NOTE: if we use 'direct link' (i.e., pbImg = pbTemp), pbImg also returns null after clearing pbTemp!
                /////////        END            //////////////

                //make sure to update dimension variables!
                iImage2_w= gdk_pixbuf_get_width(pbLogo);
                iImage2_h= gdk_pixbuf_get_height(pbLogo);
            }

            // if BOTH are being rotated, we want to keep relative positions constant; given x & y axes inverted, we must also invert scale slider values...
            if(bRotLogo && bRotMain){

                // works perfectly! we swap y-axis value for that of x-axis, and then set x-axis to the inverse of y-axis, which leads to constant relative position between base img and logo when both rotated together!
                double fTemp_slider_x = gtk_range_get_value (scale_x);
                double fTemp_slider_y = gtk_range_get_value (scale_y);
                double dif_x= 1.0 - fTemp_slider_x;

                gtk_range_set_value(scale_y, dif_x);
                gtk_range_set_value(scale_x, fTemp_slider_y);

            }

            if (bRotMain || bRotLogo)
                generate_image(NULL, false);

            break;

        case 2:
            if (bRotMain){
                GdkPixbuf *pbTemp = gdk_pixbuf_rotate_simple (pbImg, GDK_PIXBUF_ROTATE_CLOCKWISE);
                g_print("[INFO] Main image rotation requested (RIGHT) \n");


                //////////   Clean-up Code      //////////////
                g_object_unref(pbImg);                  // clear pbImg from memory (already filled after choosing Base Image),
                pbImg= gdk_pixbuf_copy (pbTemp);        // copy our rotated, temporary pixel buffer to pbImg (now empty),
                g_object_unref(pbTemp);                 // clear our temp. pxbuf. NOTE: if we use 'direct link' (i.e., pbImg = pbTemp), pbImg also returns null after clearing pbTemp!
                /////////        END            //////////////

                //make sure to update dimension variables!
                iImage1_w= gdk_pixbuf_get_width(pbImg);
                iImage1_h= gdk_pixbuf_get_height(pbImg);
            }

            if (bRotLogo){
                GdkPixbuf *pbTemp2 = gdk_pixbuf_rotate_simple (pbLogo, GDK_PIXBUF_ROTATE_CLOCKWISE);
                g_print("[INFO] Logo rotation requested (RIGHT) \n");

                 //////////   Clean-up Code      //////////////
                g_object_unref(pbLogo);                  // clear pbImg from memory (already filled after choosing Base Image),
                pbLogo= gdk_pixbuf_copy (pbTemp2);        // copy our rotated, temporary pixel buffer to pbImg (now empty),
                g_object_unref(pbTemp2);                 // clear our temp. pxbuf. NOTE: if we use 'direct link' (i.e., pbImg = pbTemp), pbImg also returns null after clearing pbTemp!
                /////////        END            //////////////

                //make sure to update dimension variables!
                iImage2_w= gdk_pixbuf_get_width(pbLogo);
                iImage2_h= gdk_pixbuf_get_height(pbLogo);
            }

            // if BOTH are being rotated, we want to keep relative positions constant; given x & y axes inverted, we must also invert scale slider values...
            if(bRotLogo && bRotMain){

                // works perfectly! we swap y-axis value for that of x-axis, and then set x-axis to the inverse of y-axis, which leads to constant relative position between base img and logo when both rotated together!
                double fTemp_slider_x = gtk_range_get_value (scale_x);
                double fTemp_slider_y = gtk_range_get_value (scale_y);
                double dif_y= 1.0 - fTemp_slider_y;

                gtk_range_set_value(scale_y, fTemp_slider_x);
                gtk_range_set_value(scale_x, dif_y);

            }

            if(bRotLogo || bRotMain)
                generate_image(NULL, false);

            break;

        case 3:

            //bRotMain = gtk_toggle_button_get_active (rotImg); <-refuses to work; apparently, check button not a 'toggle button'.
            bRotMain = !bRotMain;
            g_print("[DEBUG]: Main image rotation toggled: %d \n", bRotMain);
            break;

        case 4:

            //bRotLogo = gtk_toggle_button_get_active (rotLogo);
            bRotLogo = !bRotLogo;
            g_print("[DEBUG]: Logo rotation toggled: %d \n", bRotLogo);
            break;

        default:
             g_print("[DEBUG]: Rotation request could not be determined... \n");

    }
}
// ~~~~~~~~~~~~~ END ~~~~~~~~~~~~~~

static void activate (GtkApplication *app, gpointer user_data){
  window = gtk_application_window_new (app);
  diag = gtk_file_dialog_new();
  GtkWidget *grid = gtk_grid_new();
  GtkWidget *label = gtk_label_new(" ");
  GtkWidget *label02 = gtk_label_new(" ");
  GtkWidget *label03 = gtk_label_new("OFFSETS");
  GtkWidget *label04 = gtk_label_new("ROTATION");

  GtkWidget *label05 = gtk_label_new("[Scale]");
  GtkWidget *label06 = gtk_label_new("[Alpha]");
  GtkWidget *label07 = gtk_label_new("[X-Axis]");
  GtkWidget *label08 = gtk_label_new("[Y-Axis]");

  GtkWidget *label09 = gtk_label_new("Created by Kallan Sipple (2024)");

  path01 = gtk_label_new("No image selected...");
  path02 = gtk_label_new("No logo selected...");

  scale_x = gtk_scale_new(0, NULL);
  scale_y = gtk_scale_new(1, NULL);
  scale_s = gtk_scale_new(0, NULL);
  scale_alpha = gtk_scale_new(0, NULL);

// https://stackoverflow.com/questions/22384333/passing-additional-arguments-to-gtk-function - while not directly relevant, discussion helped better understand the 'g_signal_connect' call and how to properly send the arguments


  gtk_window_set_title (GTK_WINDOW (window), "Watermarker");
  gtk_window_set_default_size (GTK_WINDOW (window), 500, 200);  //1st, width; 2nd, height

  // set up grid.
  gtk_grid_set_column_spacing(GTK_GRID(grid),10);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 6);

  // button 01
  button = gtk_button_new_with_label ("Choose image");
  g_signal_connect (button, "clicked", G_CALLBACK (open_diag_image), NULL);

  // button 02
  button2= gtk_button_new_with_label ("Choose logo");
  g_signal_connect (button2, "clicked", G_CALLBACK (open_diag_logo), NULL);

  // button 03
 // button3= gtk_button_new_with_label ("Choose save loc.");
 // g_signal_connect (button3, "clicked", G_CALLBACK (open_diag_save), NULL);

  // button 04
  button4= gtk_button_new_with_label ("Start");
  g_signal_connect (button4, "clicked", G_CALLBACK (generate_image), true);

  // button rl
  button_rl= gtk_button_new_with_label ("L");
  g_signal_connect (button_rl, "clicked", G_CALLBACK (rotate_image), 1);

  // button rr
  button_rr= gtk_button_new_with_label ("R");
  g_signal_connect (button_rr, "clicked", G_CALLBACK (rotate_image), 2);

  rotImg = gtk_check_button_new_with_label ("MAIN");
  g_signal_connect (rotImg, "toggled", G_CALLBACK (rotate_image), 3);

  rotLogo = gtk_check_button_new_with_label ("LOGO");
  g_signal_connect (rotLogo, "toggled", G_CALLBACK (rotate_image), 4);


  g_signal_connect (scale_x, "value-changed", G_CALLBACK (generate_image), false);
  g_signal_connect (scale_y, "value-changed", G_CALLBACK (generate_image), false);
  g_signal_connect (scale_s, "value-changed", G_CALLBACK (generate_image), false);
  g_signal_connect (scale_alpha, "value-changed", G_CALLBACK (generate_image), false);




  // set elements as children of grid and arrange them
  gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), label09, 1, 20, 2, 1);        // footer
  gtk_grid_attach(GTK_GRID(grid), button, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), button2, 1, 2, 1, 1);
  //gtk_grid_attach(GTK_GRID(grid), button3, 1, 3, 1, 1);       // no longer needed, as save location now same as main image loc.
  gtk_grid_attach(GTK_GRID(grid), button4, 1, 4, 1, 1);

  gtk_grid_attach(GTK_GRID(grid), label02, 8, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), path01, 8, 1, 20, 1);      gtk_label_set_ellipsize (path01, PANGO_ELLIPSIZE_START);
  gtk_grid_attach(GTK_GRID(grid), path02, 8, 2, 20, 1);      gtk_label_set_ellipsize (path02, PANGO_ELLIPSIZE_START);

  gtk_grid_attach(GTK_GRID(grid), label03, 30, 0, 15, 1);
  gtk_grid_attach(GTK_GRID(grid), scale_s, 30, 1, 15, 1);
  gtk_grid_attach(GTK_GRID(grid), scale_alpha, 30, 2, 15, 1);
  gtk_grid_attach(GTK_GRID(grid), scale_x, 30, 3, 15, 1);
  gtk_grid_attach(GTK_GRID(grid), scale_y, 30, 4, 4, 8);

  gtk_grid_attach(GTK_GRID(grid), label05, 45, 1, 5, 1);
  gtk_grid_attach(GTK_GRID(grid), label06, 45, 2, 5, 1);
  gtk_grid_attach(GTK_GRID(grid), label07, 45, 3, 5, 1);
  gtk_grid_attach(GTK_GRID(grid), label08, 45, 6, 5, 1);


  gtk_grid_attach(GTK_GRID(grid), label04, 70, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), button_rl, 70, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), button_rr, 71, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), rotImg, 70, 2, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), rotLogo, 71, 2, 2, 1);



    imgbox = gtk_picture_new();
    gtk_widget_set_size_request(imgbox, 200, 200);
    gtk_grid_attach(GTK_GRID(grid), imgbox, 80, 0, 40, 20);


  // set grid as child of window (window can only have one child, so no attaching our many elements directly to it)
  // https://stackoverflow.com/questions/68790718/how-to-add-two-buttons-to-this-gtk4-form
  gtk_window_set_child (GTK_WINDOW (window), grid);
  gtk_window_present (GTK_WINDOW (window));

  gtk_widget_set_sensitive(button4, false); // keep 'GENERATE' button disabled if EITHER paths NULL
  gtk_range_set_range(scale_x, 0.0, 0.0);
  gtk_range_set_range(scale_y, 0.0, 0.0);
  gtk_range_set_range(scale_s, 1.0, 1.0);
  gtk_range_set_range(scale_alpha, 255.0, 255.0);

  // keeps footer tethered to bottom of window
  gtk_widget_set_vexpand (label09, GTK_ALIGN_END );
  gtk_widget_set_valign (label09, GTK_ALIGN_END );

  gtk_label_set_markup (GTK_LABEL (label09), "<span color='grey'>Created by Kallan Sipple (2024)</span>");
}

int main (int    argc, char **argv){
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}


// 32-bit jpegs don't seem able to be loaded to pixbuf, neither to texture. Interestingly, it can be loaded directly in Picture widget on startup;
// if previously converted to .png, said files give no problems. Other 32-bit images seem to work fine...
// maybe use Gelg library? (https://discourse.gnome.org/t/rendering-16-bit-image-in-gtk/5615 , https://developer.gimp.org/api/gegl/index.html#structs)

// add filter to file selector, so we can make sure only image files selectable!
