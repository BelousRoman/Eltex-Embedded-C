SECTIONS
{
	. = 0xbf000000;
	.text :
	{
		*(.text)
	}
	. = 0xbf002000;
	.data :
	{
		*(.data)
	}
}
