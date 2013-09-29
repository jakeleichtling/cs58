/* Notes:
- Start processes to generate all thumbnails and store PIDs in an array
- When viewing each thumbnail, first wait for process that generated it
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* Constants */

// Input
static const int INPUT_BUFFER_SIZE = 256;

// Output
static const int OUTPUT_SCALE_PERCENT = 25;
static const char *OUTPUT_SUFFIX = "_out.jpg";
static const char *INDEX_HTML_FILE_PATH = "index.html";

// Thumbnail
static const int THUMBNAIL_SCALE_PERCENT = 10;
static const int NO_SCALE_PERCENT = 100;
static const char *THUMBNAIL_SUFFIX = "_thumb.jpg";

/* Function Prototypes */

void generate_scaled_rotated_image(char *source_file_path, char *destination_file_path,
        int scale_percent, int rotation_degrees) ;
void display_image(char *file_path);
int prompt_rotation_degrees();

/* Function Implementations */

int main(int argc, char **argv) {
    // Calculate number of digits in argc
    int argc_digits = 1;
    int x = argc / 10;
    while (x > 0) {
        argc_digits++;
        x /= 10;
    }

    // For saving captions
    char *image_captions[argc];

    // For saving thumbnail image paths
    char *thumbnail_file_paths[argc];

    // For saving thumbnail image generation PIDs so that, for each thumbnail, we can wait
    // until it is generated before displaying it
    int thumbnail_generation_pids[argc];

    // For saving thumbnail rotation PIDs so we can wait for them before exiting
    int thumbnail_rotation_pids[argc];

    // For saving output image generation PIDs so we can wait for them before exiting
    int image_generation_pids[argc];

    // Loop through the provided image file paths and generate thumbnails
    int max_thumbnail_file_path_length = strlen(THUMBNAIL_SUFFIX) + argc_digits;
    int i;
    for (i = 1; i < argc; i++) {
        thumbnail_file_paths[i] = malloc((max_thumbnail_file_path_length + 1) * sizeof(char));
        sprintf(thumbnail_file_paths[i], "%d%s", i, THUMBNAIL_SUFFIX);

        int thumbnail_generation_pid = fork();
        if (0 == thumbnail_generation_pid) { // Child process
            generate_scaled_rotated_image(argv[i], thumbnail_file_paths[i],
                    THUMBNAIL_SCALE_PERCENT, false);
        }
        // Save the PID so we can wait on it before displaying the thumbnail
        thumbnail_generation_pids[i] = thumbnail_generation_pid;
    }

    for (i = 1; i < argc; i++) {
        // Wait for the corresponding thumbnail to be generated
        int generate_thumbnail_status;
        waitpid(thumbnail_generation_pids[i], &generate_thumbnail_status, 0);
        if (WIFEXITED(generate_thumbnail_status)
                && WEXITSTATUS(generate_thumbnail_status) != 0) {
            fprintf(stderr, "error generating thumbnail at path %s\n", thumbnail_file_paths[i]);
            exit(-1);
        }

        printf("\n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("%s\n", argv[i]);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

        // Display the thumbnail
        int display_thumbnail_pid = fork();
        if (0 == display_thumbnail_pid) { // Child process
            display_image(thumbnail_file_paths[i]);
        }

        // Ask if the image should be rotated
        int rotation_degrees = prompt_rotation_degrees();

        // Ask for the caption and save it
        char *input_buffer = malloc(INPUT_BUFFER_SIZE * sizeof(char));
        printf("Enter a caption for this image: ");
        fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);
        image_captions[i] = input_buffer;

        // Wait for the thumbnail to be closed
        printf("Close the thumbnail image to continue.\n");
        int display_thumbnail_status;
        waitpid(display_thumbnail_pid, &display_thumbnail_status, 0);
        if (WIFEXITED(display_thumbnail_status)
                && WEXITSTATUS(display_thumbnail_status) != 0) {
            fprintf(stderr, "error displaying thumbnail at path %s\n", thumbnail_file_paths[i]);
            exit(-1);
        }

        // Rotate thumbnail image (don't wait for this to complete, yet...)
        int thumbnail_rotation_pid = fork();
        if (0 == thumbnail_rotation_pid) { // Child process
            generate_scaled_rotated_image(thumbnail_file_paths[i], thumbnail_file_paths[i],
                    NO_SCALE_PERCENT, rotation_degrees);
        }
        // Save the PID so we can wait on it before exiting
        thumbnail_rotation_pids[i] = thumbnail_rotation_pid;

        // Generate output image (don't wait for this to complete, yet...)
        int image_generation_pid = fork();
        if (0 == image_generation_pid) { // Child process
            char output_image_file_path[argc_digits + strlen(OUTPUT_SUFFIX) + 1];
            sprintf(output_image_file_path, "%d%s", i, OUTPUT_SUFFIX);
            generate_scaled_rotated_image(argv[i], output_image_file_path,
                    OUTPUT_SCALE_PERCENT, rotation_degrees);
        }
        // Save the PID so we can wait on it before exiting
        image_generation_pids[i] = image_generation_pid;
    }

    // Wait for thumbnail rotation processes
    printf("Waiting for thumbnail rotations to complete...\n");
    for (i = 1; i < argc; i++) {
        int thumbnail_rotation_status;
        waitpid(thumbnail_rotation_pids[i], &thumbnail_rotation_status, 0);
        if (WIFEXITED(thumbnail_rotation_status)
                && WEXITSTATUS(thumbnail_rotation_status) != 0) {
            fprintf(stderr, "error rotating thumbnail at path %s\n", thumbnail_file_paths[i]);
            exit(-1);
        }
    }

    // Wait for output image generation processes
    printf("Waiting for image generation to complete...\n");
    for (i = 1; i < argc; i++) {
        int image_generation_status;
        waitpid(image_generation_pids[i], &image_generation_status, 0);
        if (WIFEXITED(image_generation_status)
                && WEXITSTATUS(image_generation_status) != 0) {
            fprintf(stderr, "error generating output image for image at path %s\n", argv[i]);
            exit(-1);
        }
    }

    // Generate index.html
    FILE *index_html_file = fopen(INDEX_HTML_FILE_PATH, "w");
    for (i = 1; i < argc; i++) {
        char thumbnail_file_path[strlen(THUMBNAIL_SUFFIX) + argc_digits + 1];
        sprintf(thumbnail_file_path, "%d%s", i, THUMBNAIL_SUFFIX);

        char output_image_file_path[argc_digits + strlen(OUTPUT_SUFFIX) + 1];
        sprintf(output_image_file_path, "%d%s", i, OUTPUT_SUFFIX);

        fprintf(index_html_file, "<div>");
        fprintf(index_html_file, "<a href=\"%s\"><img src=\"%s\" /></a>\n",
            output_image_file_path,
            thumbnail_file_path);
        fprintf(index_html_file, "<br/>");
        fprintf(index_html_file, "%s\n", image_captions[i]);
        fprintf(index_html_file, "</div>");
        fprintf(index_html_file, "<hr/>");
    }

    // Free thumbnail file paths and image caption strings
    for (i = 1; i < argc; i++) {
        free(thumbnail_file_paths[i]);
        free(image_captions[i]);
    }

    return 0;
}

/*
 Generates a scaled and (maybe) rotated version of the image at source_file_path
 and saves it at destination_file_path.
 */
void generate_scaled_rotated_image(char *source_file_path, char *destination_file_path,
        int scale_percent, int rotate_degrees)  {
    // Make the argument strings for scaling
    int scale_percent_digits = 1;
    int x = scale_percent / 10;
    while (x > 0) {
        scale_percent_digits++;
        x /= 10;
    }

    char scale_percent_string[scale_percent_digits + 2];
    sprintf(scale_percent_string, "%d%%", scale_percent);

    char rotate_degrees_string[4];
    sprintf(rotate_degrees_string, "%d", rotate_degrees);

    int convert_exec_result = execlp("convert", "convert", source_file_path,
            "-geometry", scale_percent_string,
            "-rotate", rotate_degrees_string,
            destination_file_path,
            NULL);
    if (convert_exec_result < 0) {
        fprintf(stderr, "generate_scaled_rotated_image exec failed\n");
        exit(-1);
    }
}

/*
  Displays the image at file_path.
  */
void display_image(char *file_path) {
    int display_exec_result = execlp("display", "display", file_path, NULL);

    if (display_exec_result < 0) {
        fprintf(stderr, "display_image exec failed\n");
        exit(-1);
    }
}

/*
 Prompts the user to specify whether the image should be rotated and returns the degrees
 by which the image should be rotated clockwise.

 (Inspired by Derek Salama.)
*/
int prompt_rotation_degrees() {
    char input_buffer[INPUT_BUFFER_SIZE];
    int degrees_options[] = { 0, 90, 270, 180 };
    int num_degrees_options = 4;

    printf("Would you like to rotate the image? Enter the integer corresponding to your choice:\n");
    printf("\t 0: No rotation\n");
    printf("\t 1: 90 degrees clockwise \n");
    printf("\t 2: 90 degrees counterclockwise \n");
    printf("\t 3: 180 degrees \n");

    fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);
    int selection_response = -1;
    sscanf(input_buffer, "%d", &selection_response);

    while (selection_response < 0 || selection_response >= num_degrees_options) {
        printf("Invalid input. Please try again.\n");
        fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);
        sscanf(input_buffer, "%d", &selection_response);
    }

    return degrees_options[selection_response];
}
