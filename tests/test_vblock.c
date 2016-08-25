#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <liblightnvm.h>

#include <CUnit/Basic.h>

static char nvm_dev_name[DISK_NAME_LEN] = "nvme0n1";
static char nvm_tgt_type[NVM_TTYPE_NAME_MAX] = "dflash";
static char nvm_tgt_name[DISK_NAME_LEN] = "nvm_vblock_tst";

int init_suite1(void)
{
	return 0;
}

int clean_suite1(void)
{
	return 0;
}

void test_VBLOCK_NEW_FREE(void)
{
	NVM_VBLOCK vblock;

	vblock = nvm_vblock_new();
	CU_ASSERT_PTR_NOT_NULL(vblock);

	nvm_vblock_free(&vblock);
	CU_ASSERT_PTR_NULL(vblock);
}

void test_VBLOCK_GET_PUT_01(void)
{
	NVM_VBLOCK vblock;
	NVM_TGT tgt;
	int ret;

	ret = nvm_mgmt_tgt_create(nvm_tgt_name, nvm_tgt_type, nvm_dev_name, 0, 0);
	CU_ASSERT(0==ret);

	tgt = nvm_tgt_open(nvm_tgt_name, 0x0);
	CU_ASSERT(tgt > 0);

	vblock = nvm_vblock_new();	/* get block from arbitrary lun */
	CU_ASSERT_PTR_NOT_NULL(vblock);
	
	ret = nvm_vblock_get(vblock, tgt);
	CU_ASSERT(0==ret);

	ret = nvm_vblock_put(vblock);
	CU_ASSERT(0==ret);

	nvm_vblock_free(&vblock);
	CU_ASSERT_PTR_NULL(vblock);

	nvm_tgt_close(tgt);
	ret = nvm_mgmt_tgt_remove(nvm_tgt_name);
	CU_ASSERT(0==ret);
}

void test_VBLOCK_GETS_PUT_01(void)
{
	NVM_VBLOCK vblock;
	NVM_TGT tgt;
	int ret;

	ret = nvm_mgmt_tgt_create(nvm_tgt_name, nvm_tgt_type, nvm_dev_name, 0, 0);
	CU_ASSERT(0==ret);

	tgt = nvm_tgt_open(nvm_tgt_name, 0x0);
	CU_ASSERT_PTR_NOT_NULL(tgt > 0);

	vblock = nvm_vblock_new();
	CU_ASSERT_PTR_NOT_NULL(vblock);		/* get block from lun 0 */
	
	ret = nvm_vblock_gets(vblock, tgt, 0, 0);
	CU_ASSERT(0==ret);

	ret = nvm_vblock_put(vblock);

	nvm_tgt_close(tgt);
	ret = nvm_mgmt_tgt_remove(nvm_tgt_name);
	CU_ASSERT(0==ret);
}

int main(int argc, char **argv)
{
	if (argc > 1) {
                if (strlen(argv[1]) > DISK_NAME_LEN) {
                        printf("Argument nvm_dev can be maximum %d characters\n",
                                                                DISK_NAME_LEN - 1);
                }
		strcpy(nvm_dev_name, argv[1]);
	}

	CU_pSuite pSuite = NULL;

	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	pSuite = CU_add_suite("nvm_vblock*", init_suite1, clean_suite1);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if (
	(NULL == CU_add_test(pSuite, "nvm_vblock_[new|free]", test_VBLOCK_NEW_FREE)) ||
	(NULL == CU_add_test(pSuite, "nvm_vblock_[get|put] 1", test_VBLOCK_GET_PUT_01)) ||
	(NULL == CU_add_test(pSuite, "nvm_vblock_[gets|put] 1", test_VBLOCK_GETS_PUT_01)) ||
	0)
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}