#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define UNIT_DURATION 0.12      // Duration of 1 unit(.)
#define DASH_UNITS 3            // Dash(-) duration in units
#define SPACE_UNITS 1           // Delay between elements (dashes or units) in one symbol in units
#define LETTER_SPACE_UNITS 3    // Delay between letters in units
#define WORD_SPACE_UNITS 7      // Delay between words in units

#define MORSE_ALPHABET_SIZE 256  // Only numbers, punctuation and latin alphabet
#define FREAD_CHUNK_SIZE 1024   // Read buffer size in bytes

#define START_DELAY_UNITS 25    // Delay in the begining of the file in units
#define END_DELAY_UNITS 25      // Delay at the end of the file in units

void init_morse_alphabet(char **morse_alphabet)
{
	morse_alphabet[65] =      strdup(".-"); // a
	morse_alphabet[66] =    strdup("-..."); // b
	morse_alphabet[67] =    strdup("-.-."); // c
	morse_alphabet[68] =     strdup("-.."); // d
	morse_alphabet[69] =       strdup("."); // e
	morse_alphabet[70] =    strdup("..-."); // f
	morse_alphabet[71] =     strdup("--."); // g
	morse_alphabet[72] =    strdup("...."); // h
	morse_alphabet[73] =      strdup(".."); // i
	morse_alphabet[74] =    strdup(".---"); // j
	morse_alphabet[75] =     strdup("-.-"); // k
	morse_alphabet[76] =    strdup(".-.."); // l
	morse_alphabet[77] =      strdup("--"); // m
	morse_alphabet[78] =      strdup("-."); // n
	morse_alphabet[79] =     strdup("---"); // o
	morse_alphabet[80] =    strdup(".--."); // p
	morse_alphabet[81] =    strdup("--.-"); // q
	morse_alphabet[82] =     strdup(".-."); // r
	morse_alphabet[83] =     strdup("..."); // s
	morse_alphabet[84] =       strdup("-"); // t
	morse_alphabet[85] =     strdup("..-"); // u
	morse_alphabet[86] =    strdup("...-"); // v
	morse_alphabet[87] =     strdup(".--"); // w
	morse_alphabet[88] =    strdup("-..-"); // x
	morse_alphabet[89] =    strdup("-.--"); // y
	morse_alphabet[90] =    strdup("--.."); // z
	
	morse_alphabet[48] =   strdup("-----"); // 0
	morse_alphabet[49] =   strdup(".----"); // 1
	morse_alphabet[50] =   strdup("..---"); // 2
	morse_alphabet[51] =   strdup("...--"); // 3
	morse_alphabet[52] =   strdup("....-"); // 4
	morse_alphabet[53] =   strdup("....."); // 5
	morse_alphabet[54] =   strdup("-...."); // 6
	morse_alphabet[55] =   strdup("--..."); // 7
	morse_alphabet[56] =   strdup("---.."); // 8
	morse_alphabet[57] =   strdup("----."); // 9

	morse_alphabet[33] =  strdup("-.-.--"); // !
	morse_alphabet[34] =  strdup(".-..-."); // "
	morse_alphabet[36] = strdup("...-..-"); // $
	morse_alphabet[38] =   strdup(".-..."); // &
	morse_alphabet[39] =  strdup(".----."); // '
	morse_alphabet[40] =   strdup("-.--."); // (
	morse_alphabet[41] =  strdup("-.--.-"); // )
	morse_alphabet[43] =   strdup(".-.-."); // +
	morse_alphabet[44] =  strdup("--..--"); // ,
	morse_alphabet[45] =  strdup("-....-"); // -
	morse_alphabet[46] =  strdup(".-.-.-"); // .
	morse_alphabet[47] =   strdup("-..-."); // /
	morse_alphabet[58] =  strdup("---..."); // :
	morse_alphabet[59] =  strdup("-.-.-."); // ;
	morse_alphabet[61] =   strdup("-...-"); // =
	morse_alphabet[63] =  strdup("..--.."); // ?
	morse_alphabet[64] =  strdup(".--.-."); // @
	morse_alphabet[95] =  strdup("..--.-"); // _
}

typedef struct
{
	int16_t *data;
	uint32_t size;
}AudioData;

typedef enum
{
	NO_ERR,
	MEMORY_ALLOC_ERR,
}audio_data_err_t;

audio_data_err_t alloc_audio_data(AudioData **audio, uint32_t size)
{
	*audio = (AudioData*)malloc(sizeof(AudioData));
	if(*audio == NULL)
	{
		return MEMORY_ALLOC_ERR;
	}
	(*audio)->data = (uint16_t*)malloc(size * sizeof(uint16_t));
	if((*audio)->data == NULL)
	{
		return MEMORY_ALLOC_ERR;
	}
	(*audio)->size = size;
	return NO_ERR;
}

void free_audio_data(AudioData **audio)
{
	if(*audio != NULL)
	{
		if((*audio)->data != NULL)
		{
			free((*audio)->data);
			(*audio)->data = NULL;
			(*audio)->size = 0;
		}
		free(*audio);
		*audio = NULL;
	}
}

int init_morse_alphabet_audio_data(AudioData **morse_alphabet_audio, AudioData *unit, AudioData *dash, AudioData *space, char **morse_alphabet)
{
	AudioData *parts_ptrs[19];
	char *letter_ptr;
	uint32_t part_counter, q, letter_len, sample_size;
	for(int i = 0;i < MORSE_ALPHABET_SIZE;i++)
	{
		letter_ptr = morse_alphabet[i];
		if(letter_ptr == NULL)
		{
			continue;
		}
		letter_len = strlen(letter_ptr);
		sample_size = 0;
		part_counter = 0;
		q = 0;
		for(int j = 0;j < letter_len;j++)
		{
			if(letter_ptr[j] == '.')
			{
				parts_ptrs[part_counter] = unit;
				sample_size += unit->size;
			}
			else if(letter_ptr[j] == '-')
			{
				parts_ptrs[part_counter] = dash;
				sample_size += dash->size;
			}
			part_counter++;
			if(j != letter_len - 1)
			{
				parts_ptrs[part_counter] = space;
				sample_size += space->size;
				part_counter++;
			}
		}
		if(alloc_audio_data(&morse_alphabet_audio[i], sample_size) != 0)
		{
			return sample_size;
		}
		for(int j = 0;j < part_counter;j++)
		{
			for(int k = 0;k < parts_ptrs[j]->size;k++)
			{
				morse_alphabet_audio[i]->data[q] = parts_ptrs[j]->data[k];
				q++;
			}
		}
	}
	return 0;
}

void init_ascii_to_audio_data_table(AudioData** ascii_to_audio_data_table, AudioData** morse_alphabet_audio, AudioData *word_space)
{

	for(int i = 0;i < MORSE_ALPHABET_SIZE;i++)
	{
		ascii_to_audio_data_table[i] = morse_alphabet_audio[i];
	}

	ascii_to_audio_data_table[32] = word_space; // ascii for 'space'
	ascii_to_audio_data_table[10] = word_space; // '\n' is treated as a space
	ascii_to_audio_data_table[96] = morse_alphabet_audio[39];
	
	// In Morse code lowercase and uppercase letters are the same
	for(int i = 0;i < 26;i++)
	{
		ascii_to_audio_data_table[97 + i] = morse_alphabet_audio[65 + i];
	}

}

void throw_memory_alloc_err(size_t alloc_size)
{
	char err_buff[100];
	snprintf(err_buff, sizeof(err_buff), "Failed to allocate %zu bytes", alloc_size);
	perror(err_buff);
	exit(1);
}


// WAV file header structure
struct __attribute__((packed)) WAVHeader {
    char chunkID[4];        // "RIFF" chunk descriptor
    uint32_t chunkSize;     // Size of the entire file minus 8 bytes
    char format[4];         // "WAVE" format
    char subchunk1ID[4];    // "fmt " subchunk descriptor
    uint32_t subchunk1Size; // Size of the fmt subchunk
    uint16_t audioFormat;   // Audio format (1 for PCM)
    uint16_t numChannels;   // Number of channels (1 for mono, 2 for stereo, etc.)
    uint32_t sampleRate;    // Sample rate (e.g., 44100 Hz)
    uint32_t byteRate;      // Byte rate (sampleRate * numChannels * bitsPerSample / 8)
    uint16_t blockAlign;    // Block align (numChannels * bitsPerSample / 8)
    uint16_t bitsPerSample; // Bits per sample (e.g., 16 bits)
    char subchunk2ID[4];    // "data" subchunk descriptor
    uint32_t subchunk2Size; // Size of the data (audio) subchunk
};

int main(int argc, char *argv[]) {
	
    char *in_file_path, *out_file_path;
	
    if(argc >= 3)
    {
	in_file_path = argv[1];
	out_file_path = argv[2];
    }
    else
    {
	printf("Input and output file paths do not specified\n");
	exit(1);
    }
	
    FILE *in_file, *out_file;
	
    // Open the input and output files
    in_file = fopen(in_file_path, "rb");
    if (!in_file) {
	char err_buff[100];
	snprintf(err_buff, sizeof(err_buff), "Error opening '%s' file", in_file_path);
        perror(err_buff);
        return 1;
    }
    out_file = fopen(out_file_path, "wb");
    if (!out_file) {
        char err_buff[100];
	snprintf(err_buff, sizeof(err_buff), "Error opening '%s' file", out_file_path);
        perror(err_buff);
        return 1;
    }
    
    const uint32_t chunk_size = FREAD_CHUNK_SIZE;
    uint8_t chunk_buffer[chunk_size];
	
    char *morse_alphabet[MORSE_ALPHABET_SIZE];
    AudioData **morse_alphabet_audio, **ascii2audio_data_table;
	
    // Parameters for the WAV file
    uint32_t sampleRate, dataSize;
    uint16_t bitsPerSample, numChannels;
    double duration, frequency;
	
    numChannels = 1;
    bitsPerSample = 16;
    frequency = 440.0;
	sampleRate = 1000;
	
	// Morse units calculation
	double amplitude;
	double unit_duration, dash_duration, space_duration, letter_space_duration, word_space_duration;
	uint32_t unit_samples_count, dash_samples_count, space_samples_count, letter_space_samples_count, word_space_samples_count;
	
	AudioData *unit, *dash, *space, *letter_space, *word_space;
	
	amplitude = pow(2, bitsPerSample - 1) - 1;
	
	unit_duration = UNIT_DURATION;
	dash_duration = UNIT_DURATION * DASH_UNITS;
	space_duration = UNIT_DURATION * SPACE_UNITS;
	letter_space_duration = UNIT_DURATION * LETTER_SPACE_UNITS;
	word_space_duration = UNIT_DURATION * WORD_SPACE_UNITS;
	
	unit_samples_count = sampleRate * unit_duration / 2;
	dash_samples_count = sampleRate * dash_duration / 2;
	space_samples_count = sampleRate * space_duration / 2;
	letter_space_samples_count = sampleRate * letter_space_duration / 2;
	word_space_samples_count = sampleRate * word_space_duration / 2;
	
	if(alloc_audio_data(&unit, unit_samples_count) == MEMORY_ALLOC_ERR)
	{
		throw_memory_alloc_err(unit_samples_count * sizeof(uint16_t));
	}
	if(alloc_audio_data(&dash, dash_samples_count) == MEMORY_ALLOC_ERR)
	{
		throw_memory_alloc_err(dash_samples_count * sizeof(uint16_t));
	}
	if(alloc_audio_data(&space, space_samples_count) == MEMORY_ALLOC_ERR)
	{
		throw_memory_alloc_err(space_samples_count * sizeof(uint16_t));
	}
	if(alloc_audio_data(&letter_space, letter_space_samples_count) == MEMORY_ALLOC_ERR)
	{
		throw_memory_alloc_err(letter_space_samples_count * sizeof(uint16_t));
	}
	if(alloc_audio_data(&word_space, word_space_samples_count) == MEMORY_ALLOC_ERR)
	{
		throw_memory_alloc_err(word_space_samples_count * sizeof(uint16_t));
	}
	
	// Samples pre-calculation
	memset(space->data, 0, space->size * sizeof(uint16_t));
	memset(letter_space->data, 0, letter_space->size * sizeof(uint16_t));
	memset(word_space->data, 0, word_space->size * sizeof(uint16_t));
	for (int i = 0; i < unit->size; ++i) {
	    double t = (double)i / sampleRate;
	    unit->data[i] = amplitude * sin(2 * M_PI * frequency * t);
	}
	for (int i = 0; i < dash->size; ++i) {
	    double t = (double)i / sampleRate;
	    dash->data[i] = amplitude * sin(2 * M_PI * frequency * t);
	}
	
	memset(morse_alphabet, 0, sizeof(morse_alphabet));
	init_morse_alphabet(morse_alphabet);
	
	morse_alphabet_audio = (AudioData**)calloc(MORSE_ALPHABET_SIZE, sizeof(AudioData*));
	if(morse_alphabet_audio == NULL)
	{
		throw_memory_alloc_err(MORSE_ALPHABET_SIZE * sizeof(AudioData*));
	}
	
	int alloc_bytes;
	if((alloc_bytes = init_morse_alphabet_audio_data(morse_alphabet_audio, unit, dash, space, morse_alphabet)) != 0)
	{
		throw_memory_alloc_err(alloc_bytes * sizeof(uint16_t));
	}
	
	ascii2audio_data_table = (AudioData**)calloc(MORSE_ALPHABET_SIZE, sizeof(AudioData*));
	if(ascii2audio_data_table == NULL)
	{
		throw_memory_alloc_err(MORSE_ALPHABET_SIZE * sizeof(AudioData*));
	}
	
	init_ascii_to_audio_data_table(ascii2audio_data_table, morse_alphabet_audio, word_space);
	
	// Main loop
	register uint32_t bytes_read, total_samples_size, i;
	register char last_letter;
	register AudioData *audio_ptr;
	
	last_letter = ' ';
	total_samples_size = 0;
	
	fseek(out_file, 44, SEEK_SET);
	
	// Write units of delay to begining
	for(int j = 0;j < START_DELAY_UNITS;j++)
	{
		fwrite(space->data, sizeof(int16_t), space->size, out_file);
	}
	
	while((bytes_read = fread(chunk_buffer, 1, chunk_size, in_file)) > 0)
	{
		for(i = 0;i < bytes_read;i++)
		{	
			audio_ptr = ascii2audio_data_table[chunk_buffer[i]];
			if(audio_ptr == NULL)
				continue;
			if(last_letter != ' ' && chunk_buffer[i] != ' ')
			{
				fwrite(letter_space->data, sizeof(int16_t), letter_space->size, out_file);
				total_samples_size += letter_space->size;
			}
			fwrite(audio_ptr->data, sizeof(int16_t), audio_ptr->size, out_file);
			total_samples_size += audio_ptr->size;
			last_letter = chunk_buffer[i];
		}
	}
	
	// Write n units of delay to the end
	for(int j = 0;j < END_DELAY_UNITS;j++)
	{
		fwrite(space->data, sizeof(int16_t), space->size, out_file);
	}
	
	total_samples_size += space->size * (START_DELAY_UNITS + END_DELAY_UNITS);
	
    dataSize = total_samples_size * numChannels * (bitsPerSample / 8);
	duration = total_samples_size / sampleRate;
	
	// Fill in the WAV header fields
    struct WAVHeader header = {
        .chunkID = {'R', 'I', 'F', 'F'},
        .chunkSize = 36 + dataSize,
        .format = {'W', 'A', 'V', 'E'},
        .subchunk1ID = {'f', 'm', 't', ' '},
        .subchunk1Size = 16,                // Size of the fmt subchunk (16 bytes)
        .audioFormat = 1,                   // PCM format
        .numChannels = numChannels,         // Number of channels
        .sampleRate = sampleRate,           // Sample rate
        .byteRate = sampleRate * numChannels * (bitsPerSample / 8),
        .blockAlign = numChannels * (bitsPerSample / 8),
        .bitsPerSample = bitsPerSample,     // Bits per sample
        .subchunk2ID = {'d', 'a', 't', 'a'},
        .subchunk2Size = dataSize
    };
	
	fseek(out_file, 0, SEEK_SET);
	
	// Write the WAV header to the file
    fwrite(&header, sizeof(header), 1, out_file);

    // Close files
    fclose(out_file);
    fclose(in_file);
	
	// Freeing memory
	free_audio_data(&unit);
	free_audio_data(&dash);
	free_audio_data(&space);
	free_audio_data(&letter_space);
	free_audio_data(&word_space);
	
	for(int i = 0;i < MORSE_ALPHABET_SIZE;i++)
	{
		free_audio_data(&(morse_alphabet_audio[i]));
	}

	free(morse_alphabet_audio);
	free(ascii2audio_data_table);

    return 0;
}