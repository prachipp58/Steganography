#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
//read and validate
Status read_and_validate_encode_args(char *argv[],EncodeInfo *encInfo)
{
	// ./a.out -e beutiful.bmp secret.txt [stego.bmp]
	// check the argv[2] extension is .bmp or not
	if((strstr(argv[2],".bmp"))!=NULL)
	{
		encInfo->src_image_fname=argv[2];
		if((strstr(argv[3],".txt"))!=NULL)
		{
			encInfo->secret_fname=argv[3];
			if(argv[4]==NULL)
			{
				encInfo->stego_image_fname="default.bmp";
			}
			else if((strstr(argv[4],".bmp"))!=NULL)
			{
				encInfo->stego_image_fname=argv[4];
			}
			else
			{
				return e_failure;
			}

		}
		else
		{
			return e_failure;
		}
	}
	else
	{
		return e_failure;
	}
	return e_success;
}
Status do_encoding(EncodeInfo *encInfo)
{
	//Encoding part
	//open the files
	if((open_files(encInfo))==e_success)
	{
		printf("Open Files Successfull\n");
                //check capacity
		if((check_capacity(encInfo))==e_success)
               {
                       printf("Check capacity Successfully\n");
		       //copy the bmp header
                       if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
		       {
			       printf("Copy bmp header Successfully\n");
			       //Encode magic string
			       if((encode_magic_string(MAGIC_STRING,encInfo))==e_success)
			       {
				       printf("Encode magic string successfully\n");
				       //Encode len of extn
				       char *extn = strrchr(encInfo->secret_fname, '.');   // finds ".txt" 
				       strcpy(encInfo->extn_secret_file, extn);            // copies ".txt" into the structure
				       encInfo->extn_size = strlen(extn);                  // 4
				       if ((encode_secret_file_extn_size(encInfo->extn_secret_file,encInfo)) == e_success)
				       {
					       printf("Encoded extn size successfully\n");
					       //Encode the extn
					       if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
					       {						       
						       printf("Encoded extn successfully\n");
						       //Encode the secret data size
						       if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
						       {
							       printf("Encoded secret file size successfully\n");
							       //Encode the secret data
							       if(encode_secret_file_data(encInfo) == e_success)
							       {
								       printf("Encoded secret file data successfully\n");
								       //copy remaining
								       if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
								       {
									       printf("Copied remaining data successfully\n");
								       }
								       else
								       {
									       return e_failure;
								       }
							       }
							       else
							       {
								       return e_failure;
							       }
						       }
						       else
						       {
							       return e_failure;
						       }
					       }
					       else
					       {
						       return e_failure;
					       }

				       }
				       else
				       {
					       return e_failure;
				       }
			       }
			       else
			       {
				       return e_failure;
			       }

		       }
		       else
		       {
			       return e_failure;
		       }
               }
               else
               {
                       return e_failure;
               }

	}
	else
	{
		return e_failure;
	}

	//close files
	fclose(encInfo->fptr_src_image);     // beautiful.bmp
        fclose(encInfo->fptr_secret);        // secret.txt
        fclose(encInfo->fptr_stego_image);   // default.bmp
	return e_success;

}
//to check whether source file .bmp has capacity to store data or not
Status check_capacity(EncodeInfo *encInfo)
{
	//Size of beuatiful.bmp > (16+32+32+32+(sizeof secret file*8))
	encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
	encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);
	char *extn = strrchr(encInfo->secret_fname, '.');
	if (extn == NULL)
	{
	    	printf("Error: secret file has no extension\n");
	    	return e_failure;
	}
	strcpy(encInfo->extn_secret_file, extn);
	encInfo->extn_size = strlen(extn);
       	printf("Image size : %d\n", encInfo->image_capacity);
      	printf("Sec file size : %ld\n", encInfo->size_secret_file);
	if(encInfo->image_capacity >= (16 + 32 + (encInfo->extn_size*8) + 32 + (encInfo->size_secret_file*8)))
	{
		return e_success;
	}
	else
	{
		return e_failure;
	}
}
uint get_file_size(FILE *fptr)
{
	//set yuor file off set to 0th position
	//find the size of file and return the size
	fseek(fptr, 0, SEEK_END); 
	uint size =ftell(fptr);
	rewind(fptr);
	return size;
}
//copy bmp header 54 bytes
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
	char arr[54];
	fseek(fptr_src_image,0,SEEK_SET);
	fread(arr,54,1,fptr_src_image);
	fwrite(arr,54,1,fptr_dest_image);
	return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
	for(int i=0;i<8;i++)
	{
		if((data & (1<<i)))
		{
			image_buffer[i]=(image_buffer[i] & 0XFE) | 1;
		}
		else
		{
			image_buffer[i]=(image_buffer[i] & 0XFE) | 0;
		}
	}
	return e_success;
}
Status encode_int_to_lsb(char data, char *image_buffer)
{
        for(int i=0;i<32;i++)
        {
                if((data & (1<<i)))
                {
                        image_buffer[i]=(image_buffer[i] & 0XFE) | 1;
                }
                else
                {
                        image_buffer[i]=(image_buffer[i] & 0XFE) | 0;
                }
        }
        return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
	if(encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_success)
	{
		return e_success;
	}
	return e_failure;
}
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	char buffer[8];
	for(int i=0;i<size;i++)
	{
		fread(buffer,8,1,fptr_src_image);
		if((encode_byte_to_lsb(data[i],buffer)) != e_success)
		{
			return e_failure;
		}
		fwrite(buffer,8,1,fptr_stego_image);
	}
	return e_success;
}
Status encode_secret_file_extn_size(const char *file_extn, EncodeInfo *encInfo)
{
    // find the length of the extension and store it in extn_size
    encInfo->extn_size = strlen(file_extn);

    char buffer[32];                                        // declare one buffer[32]
    fread(buffer, 32, 1, encInfo->fptr_src_image);          // read 32 bytes from the src file
    encode_int_to_lsb(encInfo->extn_size, buffer);          // call int to lsb function
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);       // write 32 bytes to the stego file
    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for (int i = 0; file_extn[i] != '\0'; i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_int_to_lsb((int)file_size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    char buffer[8];

    rewind(encInfo->fptr_secret);

    for (uint i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(&ch, 1, 1, encInfo->fptr_secret);
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(ch, buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}
