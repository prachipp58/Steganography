#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <string.h>

int main(int argc, char *argv[])
{
	EncodeInfo encode;
	DecodeInfo decode;
	//validate the cmd args
	//./a.out -e beautiful.bmp secret.txt
	//-e encoding
	//-d decoding
	//arg count should be at least 4 if not printf error
	int ret=check_operation_type(argv);
	if(ret==e_encode)
	{
		//encoding
		printf("Selected Encoding..\n");
		if((read_and_validate_encode_args(argv,&encode))==e_success)
		{
			printf("Read and validate Success\n");
			if((do_encoding(&encode)==e_success))
			{
				printf("Encoding Successfull\n");
			}
			else
			{
				printf("Encoding Failure\n");
			}
		}
		else
		{
			printf("Read and validate Failure\n");
			return 0;
		}
	}
	else if(ret==e_decode)
	{
		//decoding
		printf("Seleted Decoding..\n");
		if((read_and_validate_decode_args(argv,&decode))==e_success)
                {
                        printf("Read and validate Success\n");
                        if((do_decoding(&decode)==e_success))
                        {
                                printf("Decoding Successfull\n");
                        }
                        else
                        {
                                printf("Decoding Failure\n");
                        }
                }
                else
                {
                        printf("Read and validate Failure\n");
                        return 0;
                }
		
	}
	else
	{
		printf("Invalid Option\n");
	}
	return 0;
}
OperationType check_operation_type(char *argv[])
{
	if((strcmp(argv[1],"-e"))==0)
	{
		return e_encode;
	}
	else if((strcmp(argv[1],"-d"))==0)
	{
		return e_decode;
	}
	else
	{
		return e_unsupported;
	}
}
