#ifndef CHARACTERS_ATARIST8X16SYSTEMFONT_H
#define CHARACTERS_ATARIST8X16SYSTEMFONT_H

int getAtariST8x16SystemFontCharacter(int index, int y);
int getAtariST8x16SystemFontAdvance(int index);
int getAtariST8x16SystemFontKerning(int left, int right);
#define FONT_ATARIST8X16SYSTEMFONT_WIDTH 8
#define FONT_ATARIST8X16SYSTEMFONT_HEIGHT 16

#endif
