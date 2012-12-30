Vector3 up(0, 1, 0);
Vector3 right(1, 0, 0);
Vector3 back(0, 0, 1);

double rotatingX = 0;
double rotatingY = 0;
double rotatingZ = 0;

char *imageData = 0;
int image_width;
int image_height;

GLuint textureId;

void draw()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float R = 10000;

    // Set lookat

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt
    (
        0, 0, 0,
        -back.x, -back.y, -back.z,
        up.x, up.y, up.z
    );


    // Draw that skybox

    glColor3f(1, 1, 1);
    double pW = image_width / 4.0; // panel width
    double pH = image_height / 3.0; // panel height

    glBegin(GL_QUADS);
    {
        // left
        glTexCoord2f(0, pH);
        glVertex3f(-R, R, R);
        glTexCoord2f(0, 2 * pH);
        glVertex3f(-R, -R, R);
        glTexCoord2f(pW, 2 * pH);
        glVertex3f(-R, -R, -R);
        glTexCoord2f(pW, pH);
        glVertex3f(-R, R, -R);

        // right

        glTexCoord2f(3 * pW, pH);
        glVertex3f(R, R, R);
        glTexCoord2f(3 * pW, 2 * pH);
        glVertex3f(R, -R, R);
        glTexCoord2f(2 * pW, 2 * pH);
        glVertex3f(R, -R, -R);
        glTexCoord2f(2 * pW, pH);
        glVertex3f(R, R, -R);

        // bottom
        glTexCoord2f(2 * pW, 3 * pH);
        glVertex3f(R, -R, R);
        glTexCoord2f(pW, 3 * pH);
        glVertex3f(-R, -R, R);
        glTexCoord2f(pW, 2 * pH);
        glVertex3f(-R, -R, -R);
        glTexCoord2f(2 * pW, 2 * pH);
        glVertex3f(R, -R, -R);

        // top
        glTexCoord2f(2 * pW, 0);
        glVertex3f(R, R, R);
        glTexCoord2f(pW, 0);
        glVertex3f(-R, R, R);
        glTexCoord2f(pW, pH);
        glVertex3f(-R, R, -R);
        glTexCoord2f(2 * pW, pH);
        glVertex3f(R, R, -R);

        // back
        glTexCoord2f(2 * pW, pH);
        glVertex3f(R, R, -R);
        glTexCoord2f(pW, pH);
        glVertex3f(-R, R, -R);
        glTexCoord2f(pW, 2 * pH);
        glVertex3f(-R, -R, -R);
        glTexCoord2f(2 * pW, 2 * pH);
        glVertex3f(R, -R, -R);

        // back
        glTexCoord2f(3 * pW, pH);
        glVertex3f(R, R, R);
        glTexCoord2f(4 * pW, pH);
        glVertex3f(-R, R, R);
        glTexCoord2f(4 * pW, 2 * pH);
        glVertex3f(-R, -R, R);
        glTexCoord2f(3 * pW, 2 * pH);
        glVertex3f(R, -R, R);


    }
    glEnd();
}

bool handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // check for messages
        switch (event.type)
        {
            // exit if the window is closed
            case SDL_QUIT:
                return true;
                break;
            // Handle rotation
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_LEFT: rotatingX = 1; break;
                    case SDLK_RIGHT: rotatingX = -1; break;
                    case SDLK_UP: rotatingY = 1; break;
                    case SDLK_DOWN: rotatingY = -1; break;
                    case SDLK_q: rotatingZ = 1; break;
                    case SDLK_e: rotatingZ = -1; break;
                    default:;
                }
            }
            break;
            case SDL_KEYUP:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_LEFT: rotatingX = 0; break;
                    case SDLK_RIGHT: rotatingX = 0; break;
                    case SDLK_UP: rotatingY = 0; break;
                    case SDLK_DOWN: rotatingY = 0; break;
                    case SDLK_q: rotatingZ = 0; break;
                    case SDLK_e: rotatingZ = 0; break;
                    default:;
                }
            }
            break;


        }

    } // end of message processing
    return false;
}

void applyRotations()
{
    const double ANGLE_STEP = 0.01;
    Matrix xRotation = createRotationMatrix(up, ANGLE_STEP * rotatingX);
    Matrix yRotation = createRotationMatrix(right, ANGLE_STEP * rotatingY);
    Matrix zRotation = createRotationMatrix(back, ANGLE_STEP * rotatingZ);

    Matrix combined = xRotation % yRotation % zRotation;

    up = transformVector(combined, up);
    right = transformVector(combined, right);
    back = transformVector(combined, back);

    fix3VectorSystem(right, up, back);
}

bool checkParameters(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage cpv <filename>\n");
        return false;
    }
    return true;
}

void onPngError(png_structp pngStruct, png_const_charp errorMsg)
{
    printf("%s\n", errorMsg);

}

bool loadImage(const char *fn)
{
    const int PNGSIGSIZE = 8;
    bool retVal = true;
    png_structp pngReadStruct = 0;
    png_infop pngInfoStruct = 0;

    FILE *f = fopen(fn, "rb");
    png_bytep *rows;
    png_byte signature[PNGSIGSIZE];

    if (!f)
    {
        printf("Unable to open the file!\n");
        retVal = false;
        goto cleanup;
    }

    // Validate the file.
    if (!fread(signature, PNGSIGSIZE, 1, f))
    {
        printf("Unable to read the signature.\n");
        retVal = false;
        goto cleanup;
    }

    if (png_sig_cmp(signature, 0, PNGSIGSIZE))
    {
        printf("That's not a PNG file.\n");
        printf("You read this:\n");
        for (int i = 0; i < PNGSIGSIZE; i++)
        {
            printf("%02x ", signature[i]);
        }
        printf("\n");
        retVal = false;
        goto cleanup;
    }

    // Create structs
    pngReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, onPngError, 0);
    if (!pngReadStruct)
    {
        printf("Unable to initialize PNG read struct.\n");
        retVal = false;
        goto cleanup;
    }

    pngInfoStruct = png_create_info_struct(pngReadStruct);
    if (!pngInfoStruct)
    {
        printf("Unable to create PNG info struct.\n");
        retVal = false;
        goto cleanup;
    }

    // Set up it to read from our file.

    png_init_io(pngReadStruct, f);

    // Read the image

    png_set_sig_bytes(pngReadStruct, 8);

    png_set_palette_to_rgb(pngReadStruct);
    png_read_png(pngReadStruct, pngInfoStruct, PNG_TRANSFORM_IDENTITY, 0);

    // Dump out some info for testing:

    image_width = png_get_image_width(pngReadStruct, pngInfoStruct);
    image_height = png_get_image_height(pngReadStruct, pngInfoStruct);

    printf("Image width is: %d\n", (int)image_width);
    printf("Image height is: %d\n", (int)image_height);

    rows = png_get_rows(pngReadStruct, pngInfoStruct);

    imageData = (char*)malloc(3 * image_width * image_height);

    for (int i = 0; i < image_height; i++)
    {
        memcpy(imageData + 3 * image_width * i, rows[i], 3 * image_width);
    }

cleanup:

    if (f)
    {
        fclose(f);
    }
    if (pngReadStruct)
    {
        png_destroy_read_struct(&pngReadStruct, 0, 0);
    }
    return retVal;
}

int main ( int argc, char* argv[])
{
    // Check parameters

    if (!checkParameters(argc, argv))
    {
        return 0;
    }

    // Load image.

    if (!loadImage(argv[1]))
    {
        return 0;
    }

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // Setting OpenGL  attributes

    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(1024, 768, 16,
                                           SDL_HWSURFACE | SDL_OPENGL);
    if ( !screen )
    {
        printf("Unable to set 1024×768 video: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize modes

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_RECTANGLE);

    // Create textures

    glGenTextures(1, &textureId);
    if (glGetError())
    {
        printf("Unable to generate the texture.\n");
        return 1;
    }
    glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
    if (glGetError())
    {
        printf("Unable to bind the texture.\n");
        return 1;
    }
    glTexImage2D
    (
        GL_TEXTURE_RECTANGLE,
        0,
        GL_RGB8,
        image_width,
        image_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        imageData
    );
    if (glGetError())
    {
        printf("Unable to load the texture.\n");
        return 1;
    }

    // Initialize gl matrixes

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 4.0/3.0, 1, 100000.0);

    // program main loop
    for(;;)
    {
        // message processing loop
        if (handleEvents()) break;
        // Do calculations
        applyRotations();


        // DRAWING STARTS HERE

        draw();

        // DRAWING ENDS HERE

        // finally, update the screen
        SDL_GL_SwapBuffers();
    } // end main loop

    free(imageData);

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}
