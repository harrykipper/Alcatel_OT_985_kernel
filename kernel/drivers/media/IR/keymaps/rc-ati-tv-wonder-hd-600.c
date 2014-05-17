

#include <media/rc-map.h>


static struct ir_scancode ati_tv_wonder_hd_600[] = {
	{ 0x00, KEY_RECORD},		/* Row 1 */
	{ 0x01, KEY_PLAYPAUSE},
	{ 0x02, KEY_STOP},
	{ 0x03, KEY_POWER},
	{ 0x04, KEY_PREVIOUS},	/* Row 2 */
	{ 0x05, KEY_REWIND},
	{ 0x06, KEY_FORWARD},
	{ 0x07, KEY_NEXT},
	{ 0x08, KEY_EPG},		/* Row 3 */
	{ 0x09, KEY_HOME},
	{ 0x0a, KEY_MENU},
	{ 0x0b, KEY_CHANNELUP},
	{ 0x0c, KEY_BACK},		/* Row 4 */
	{ 0x0d, KEY_UP},
	{ 0x0e, KEY_INFO},
	{ 0x0f, KEY_CHANNELDOWN},
	{ 0x10, KEY_LEFT},		/* Row 5 */
	{ 0x11, KEY_SELECT},
	{ 0x12, KEY_RIGHT},
	{ 0x13, KEY_VOLUMEUP},
	{ 0x14, KEY_LAST},		/* Row 6 */
	{ 0x15, KEY_DOWN},
	{ 0x16, KEY_MUTE},
	{ 0x17, KEY_VOLUMEDOWN},
};

static struct rc_keymap ati_tv_wonder_hd_600_map = {
	.map = {
		.scan    = ati_tv_wonder_hd_600,
		.size    = ARRAY_SIZE(ati_tv_wonder_hd_600),
		.ir_type = IR_TYPE_UNKNOWN,	/* Legacy IR type */
		.name    = RC_MAP_ATI_TV_WONDER_HD_600,
	}
};

static int __init init_rc_map_ati_tv_wonder_hd_600(void)
{
	return ir_register_map(&ati_tv_wonder_hd_600_map);
}

static void __exit exit_rc_map_ati_tv_wonder_hd_600(void)
{
	ir_unregister_map(&ati_tv_wonder_hd_600_map);
}

module_init(init_rc_map_ati_tv_wonder_hd_600)
module_exit(exit_rc_map_ati_tv_wonder_hd_600)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mauro Carvalho Chehab <mchehab@redhat.com>");
