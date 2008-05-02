#ifndef	_LINUX_ASCII_H
#define	_LINUX_ASCII_H

/* ASCII control code definitions */

#define	ASCII_NUL	0	/* null (end of string) */
#define	ASCII_SOH	1	/* start of heading	*/
#define	ASCII_STX	2	/* start of text	*/
#define	ASCII_ETX	3	/* end of text		*/
#define	ASCII_EOT	4	/* end of transmission	*/
#define	ASCII_ENQ	5	/* enquiry		*/
#define	ASCII_ACK	6	/* acknowledge		*/
#define	ASCII_BEL	7	/* bell			*/
#define	ASCII_BS	8	/* backspace		*/
#define	ASCII_HT	9	/* horizontal tab	*/
#define	ASCII_TAB	9	/* horizontal tab	*/

#define	ASCII_LF	10	/* line feed	        */
#define	ASCII_VT	11	/* vertical tab		*/
#define	ASCII_FF	12	/* form feed		*/
#define	ASCII_CR	13	/* carriage return	*/
#define	ASCII_SO	14	/* shift out		*/
#define	ASCII_SI	15	/* shift in		*/
#define	ASCII_DLE	16	/* data line escape	*/
#define	ASCII_DC1	17	/* dev-ctrl 1 (X-ON)	*/
#define	ASCII_DC2	18	/* dev-ctrl 2       	*/
#define	ASCII_DC3	19	/* dev-ctrl 3 (X-OFF)	*/

#define	ASCII_DC4	20	/* dev-ctrl 4       	*/
#define	ASCII_NAK	21	/* negative acknowledge	*/
#define	ASCII_SYN	22	/* synchronous idle	*/
#define	ASCII_ETB	23	/* end of transmit block*/
#define	ASCII_CAN	24	/* cancel		*/
#define	ASCII_EM	25	/* end of medium	*/
#define	ASCII_SUB	26	/* substitute		*/
#define	ASCII_ESC	27	/* escape		*/
#define	ASCII_FS	28	/* file separator	*/
#define	ASCII_GS	29	/* group separator	*/

#define	ASCII_RS	30	/* record separator	*/
#define	ASCII_US	31	/* unit separator	*/

#endif	/* _LINUX_ASCII_H */
