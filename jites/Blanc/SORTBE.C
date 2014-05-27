/* << el2007 */
int sort_function(const void *_a, const void *_b)
{
	return(strcmp(_a, _b));
}

void SortFiles(void)
{
	qsort((void *)stru_fichier, count, 14, sort_function);
}
/* el2007 >> */
