/* ----------------------------------------------------------------------------
**	I2C driver for DDC2 communication	
** ----------------------------------------------------------------------------
** 	This driver is assembled from the Linux kernel code and XFree sources.
**	This file is distributed under the terms and conditions of the
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: I2C.c,v $
**	Revision 1.2  2003/03/11 07:56:55  fspacek
**	- adjust conditions since KGI_EOK == 0
**	
**	Revision 1.1  2003/02/09 10:58:22  foske
**	Initial commit. Code should work, though my monitors don't respond :(
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jos_Hulzink
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.2 $"

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif

#include <kgi/module.h>

/* This is the default I2CUDelay function if not supplied by the driver.
 * High level I2C interfaces implementing the bus protocol in hardware
 * should supply this function too.
 *
 * Delay execution at least usec microseconds.
 * All values 0 to 1e6 inclusive must be expected.
 */

static void I2CUDelay(void *i2c_io, kgi_u_t usec) {
	kgi_udelay(3*usec);
}

/* Most drivers will register just with GetBits/PutBits functions.
 * The following functions implement a software I2C protocol
 * by using the promitive functions given by the driver.
 * ================================================================
 *
 * It is assumed that there is just one master on the I2C bus, therefore
 * there is no explicit test for conflits.
 */

#define RISEFALLTIME 2 /* usec, actually 300 to 1000 ns according to the i2c specs */

/* Some devices will hold SCL low to slow down the bus or until 
 * ready for transmission.
 *
 * This condition will be noticed when the master tries to raise
 * the SCL line. You can set the timeout to zero if the slave device
 * does not support this clock synchronization.
 */

static kgi_error_t
I2CRaiseSCL(kgim_i2c_io_t *i2c_io, kgi_u_t sda, kgi_u_t timeout)
{
    kgi_u_t scl;
    	kgi_s_t i;
    
    i2c_io->PutBits(i2c_io->chipset_io, 1, sda);
    i2c_io->UDelay(i2c_io, i2c_io->RiseFallTime);

    for (i = timeout; i > 0; i -= i2c_io->RiseFallTime) {
	i2c_io->GetBits(i2c_io->chipset_io, &scl, &sda);
	if (scl) break;
	i2c_io->UDelay(i2c_io, i2c_io->RiseFallTime);
    }

    if (i <= 0) {
	KRN_DEBUG(1,"[I2CRaiseSCL(%d, %d) timeout]", sda, timeout);    
	return KGI_ERRNO(MONITOR,UNKNOWN);
    }

    return -KGI_EOK;
}

/* Send a start signal on the I2C bus. The start signal notifies
 * devices that a new transaction is initiated by the bus master.
 *
 * The start signal is always followed by a slave address.
 * Slave addresses are 8+ bits. The first 7 bits identify the
 * device and the last bit signals if this is a read (1) or
 * write (0) operation.
 *
 * There may be more than one start signal on one transaction.
 * This happens for example on some devices that allow reading
 * of registers. First send a start bit followed by the device
 * address (with the last bit 0) and the register number. Then send
 * a new start bit with the device address (with the last bit 1)
 * and then read the value from the device.
 *
 * Note this is function does not implement a multiple master
 * arbitration procedure.
 */

static kgi_error_t
I2CStart(kgim_i2c_io_t *i2c_io, kgi_u_t timeout)
{
    kgi_u_t scl, sda;
    kgi_s_t i;
    i2c_io->PutBits(i2c_io->chipset_io, 1, 1);
    i2c_io->UDelay(i2c_io, i2c_io->RiseFallTime);
    
    //KRN_DEBUG(2,"i2c: I2CStart");

    for (i = timeout; i > 0; i -= i2c_io->RiseFallTime) {
	i2c_io->GetBits(i2c_io->chipset_io, &scl, &sda);
	if (scl) break;
	i2c_io->UDelay(i2c_io, i2c_io->RiseFallTime);
    }

    if (i <= 0) { 
	KRN_DEBUG(1,"i2c: <[I2CStart(%d) timeout]", timeout);
	return KGI_ERRNO(MONITOR,UNKNOWN);
    }

    i2c_io->PutBits(i2c_io->chipset_io, 1, 0);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);
    i2c_io->PutBits(i2c_io->chipset_io, 0, 0);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);

    return -KGI_EOK;
}

/* This is the default I2CStop function if not supplied by the driver.
 *
 * Signal devices on the I2C bus that a transaction on the
 * bus has finished. There may be more than one start signal
 * on a transaction but only one stop signal.
 */

static kgi_error_t I2CStop(I2CDevRec * d) {

    kgim_i2c_io_t *i2c_io = (kgim_i2c_io_t*)d->pI2CBus;
    i2c_io->PutBits(i2c_io->chipset_io, 0, 0);
    i2c_io->UDelay(i2c_io, i2c_io->RiseFallTime);

    i2c_io->PutBits(i2c_io->chipset_io, 1, 0);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);
    i2c_io->PutBits(i2c_io->chipset_io, 1, 1);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);

    //KRN_DEBUG(2,"I2CStop");
    return -KGI_EOK;
}

/* Write/Read a single bit to/from a device.
 * Return KGI_ERRNO(MONITOR,UNKNOWN) if a timeout occurs.
 */

static kgi_error_t I2CWriteBit(kgim_i2c_io_t *i2c_io, kgi_u_t sda, kgi_u_t timeout)
{
    kgi_error_t r;
    KRN_DEBUG(4,"I2CWriteBit %d", sda);

    i2c_io->PutBits(i2c_io->chipset_io, 0, sda);
    i2c_io->UDelay(i2c_io, i2c_io->RiseFallTime);

    r = I2CRaiseSCL(i2c_io, sda, timeout);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);

    i2c_io->PutBits(i2c_io->chipset_io, 0, sda);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);

    return r;
}

static kgi_error_t I2CReadBit(kgim_i2c_io_t *i2c_io, kgi_u_t *psda, kgi_u_t timeout) {
    kgi_error_t r;
    kgi_u_t scl;
    r = I2CRaiseSCL(i2c_io, 1, timeout);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);

    i2c_io->GetBits(i2c_io->chipset_io, &scl, psda);

    i2c_io->PutBits(i2c_io->chipset_io, 0, 1);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);

    KRN_DEBUG(4,"I2CReadBit %d", *psda);
    return r;
}

/* This is the default I2CPutByte function if not supplied by the driver.
 *
 * A single byte is sent to the device.
 * The function returns KGI_ERRNO(MONITOR,UNKNOWN) if a timeout occurs, you should send 
 * a stop condition afterwards to reset the bus.
 *
 * A timeout occurs,
 * if the slave pulls SCL to slow down the bus more than ByteTimeout usecs,
 * or slows down the bus for more than BitTimeout usecs for each bit,
 * or does not send an ACK bit (0) to acknowledge the transmission within
 * AcknTimeout usecs, but a NACK (1) bit.
 *
 * AcknTimeout must be at least i2c_io->HoldTime, the other timeouts can be
 * zero according to the comment on I2CRaiseSCL.
 */

static kgi_error_t I2CPutByte(I2CDevRec * d, kgi_u8_t data)
{
    kgi_error_t r;
    kgi_u_t scl, sda;
    kgi_s_t i;
    kgim_i2c_io_t *i2c_io = (kgim_i2c_io_t*)d->pI2CBus;
	KRN_DEBUG(4,"Byte: %x, HoldTime %d", data, i2c_io->HoldTime);
    
    if (-KGI_EOK != I2CWriteBit(i2c_io, (data >> 7) & 1, d->ByteTimeout))
	return KGI_ERRNO(MONITOR,UNKNOWN);	

    for (i = 6; i >= 0; i--) 
	if (-KGI_EOK != I2CWriteBit(i2c_io, (data >> i) & 1, d->BitTimeout))
	    return KGI_ERRNO(MONITOR,UNKNOWN);

    i2c_io->PutBits(i2c_io->chipset_io, 0, 1);
    i2c_io->UDelay(i2c_io, i2c_io->RiseFallTime);

    r = I2CRaiseSCL(i2c_io, 1, i2c_io->HoldTime);

    if (-KGI_EOK == r) {
    	for (i = d->AcknTimeout; i > 0; i -= i2c_io->HoldTime) {
	    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);
	    i2c_io->GetBits(i2c_io->chipset_io, &scl, &sda);
	    if (sda == 0) break;
	}

	if (i <= 0) {
	    KRN_DEBUG(1,"[I2CPutByte(0x%02x, %d, %d, %d) timeout]", 
				       data, d->BitTimeout, 
				       d->ByteTimeout, d->AcknTimeout);
	    r = KGI_ERRNO(MONITOR,UNKNOWN);
	}
    }

    i2c_io->PutBits(i2c_io->chipset_io, 0, 1);
    i2c_io->UDelay(i2c_io, i2c_io->HoldTime);

    return r;
}

/* This is the default I2CGetByte function if not supplied by the driver.
 *
 * A single byte is read from the device.
 * The function returns KGI_ERRNO(MONITOR,UNKNOWN) if a timeout occurs, you should send
 * a stop condition afterwards to reset the bus.
 * 
 * A timeout occurs,
 * if the slave pulls SCL to slow down the bus more than ByteTimeout usecs,
 * or slows down the bus for more than i2c_io->BitTimeout usecs for each bit.
 *
 * ByteTimeout must be at least i2c_io->HoldTime, the other timeouts can be
 * zero according to the comment on I2CRaiseSCL.
 *
 * For the <last> byte in a sequence the acknowledge bit NACK (1), 
 * otherwise ACK (0) will be sent.
 */

static kgi_error_t
I2CGetByte(I2CDevRec * d, kgi_u8_t *data, kgi_u8_t last)
{
    kgi_u_t sda;
    kgi_s_t i;
    kgim_i2c_io_t *i2c_io = (kgim_i2c_io_t*)d->pI2CBus;

    i2c_io->PutBits(i2c_io->chipset_io, 0, 1);
    i2c_io->UDelay(i2c_io, i2c_io->RiseFallTime);

    if (-KGI_EOK != I2CReadBit(i2c_io, &sda, d->ByteTimeout))
	return KGI_ERRNO(MONITOR,UNKNOWN);

    *data = (sda > 0) << 7;

    for (i = 6; i >= 0; i--)
	if (-KGI_EOK != I2CReadBit(i2c_io, &sda, d->BitTimeout))
	    return KGI_ERRNO(MONITOR,UNKNOWN);
	else
	    *data |= (sda > 0) << i;

    if (-KGI_EOK != I2CWriteBit(i2c_io, last ? 1 : 0, d->BitTimeout))
	return KGI_ERRNO(MONITOR,UNKNOWN);

    KRN_DEBUG(2,"R%02x%c ", (int) *data, last ? '+' : '-');

    return -KGI_EOK;
}

/* This is the default I2CAddress function if not supplied by the driver.
 *
 * It creates the start condition, followed by the d->SlaveAddr.
 * Higher level functions must call this routine rather than
 * I2CStart/PutByte because a hardware I2C master may not be able 
 * to send a slave address without a start condition.
 *
 * The same timeouts apply as with I2CPutByte and additional a
 * StartTimeout, similar to the ByteTimeout but for the start 
 * condition.
 *
 * In case of a timeout, the bus is left in a clean idle condition.
 * I. e. you *must not* send a Stop. If this function succeeds, you *must*.
 *
 * The slave address format is 16 bit, with the legacy _8_bit_ slave address
 * in the least significant byte. This is, the slave address must include the
 * R/_W flag as least significant bit.
 *
 * The most significant byte of the address will be sent _after_ the LSB, 
 * but only if the LSB indicates:
 * a) an 11 bit address, this is LSB = 1111 0xxx.
 * i2c_io) a 'general call address', this is LSB = 0000 000x - see the I2C specs
 *    for more.
 */

static kgi_error_t I2CAddress(I2CDevRec * d, kgi_u_t addr) {
	
    kgim_i2c_io_t *i2c_io = (kgim_i2c_io_t*)d->pI2CBus;
    
    if (-KGI_EOK == I2CStart(i2c_io, d->StartTimeout)) {
	if (-KGI_EOK == I2CPutByte(d, addr & 0xFF)) {
	    if ((addr & 0xF8) != 0xF0 &&
		(addr & 0xFE) != 0x00)
		return -KGI_EOK;

	    if (-KGI_EOK == I2CPutByte(d, (addr >> 8) & 0xFF))
		return -KGI_EOK;
	}
	I2CStop(d);
    }
    return KGI_ERRNO(MONITOR,UNKNOWN);
}

/* These are the hardware independent I2C helper functions.
 * ========================================================
 */

/* Function for probing. Just send the slave address 
 * and return true if the device responds. The slave address
 * must have the lsb set to reflect a read (1) or write (0) access.
 * Don't expect a read- or write-only device will respond otherwise.
 */

static kgi_error_t
i2c_ProbeAddress(kgim_i2c_io_t *i2c_io, kgi_u_t addr)
{
    kgi_u_t r;
    I2CDevRec d;
	
    d.DevName = "Probing";
    d.BitTimeout = i2c_io->BitTimeout;
    d.ByteTimeout = i2c_io->ByteTimeout;
    d.AcknTimeout = i2c_io->AcknTimeout;
    d.StartTimeout = i2c_io->StartTimeout;
    d.SlaveAddr = addr;
    d.pI2CBus = i2c_io;

    KRN_DEBUG(1,"Probing address 0x%x", addr);
    r = i2c_io->Address(&d, addr);

    return r;
}

/* All functions below are related to devices and take the
 * slave address and timeout values from an I2CDevRec. They
 * return KGI_ERRNO(MONITOR,UNKNOWN) in case of an error (presumably a timeout).
 */

/* General purpose read and write function.
 *
 * 1st, if nWrite > 0
 *   Send a start condition
 *   Send the slave address (1 or 2 bytes) with write flag
 *   Write n bytes from WriteBuffer
 * 2nd, if nRead > 0
 *   Send a start condition [again]
 *   Send the slave address (1 or 2 bytes) with read flag
 *   Read n bytes to ReadBuffer
 * 3rd, if a Start condition has been successfully sent,
 *   Send a Stop condition.
 *
 * The functions exits immediately when an error occures,
 * not proceeding any data left. However, step 3 will
 * be executed anyway to leave the bus in clean idle state. 
 */

static kgi_error_t
I2CWriteRead(I2CDevRec * d,
		 kgi_u8_t *WriteBuffer, kgi_u_t nWrite,
		 kgi_u8_t *ReadBuffer,  kgi_u_t nRead) 
{
    kgi_error_t r = -KGI_EOK;
    kgim_i2c_io_t *i2c_io = d->pI2CBus;
    kgi_u_t s = 0;

    if (-KGI_EOK == r && nWrite > 0) {
	r = i2c_io->Address(d, d->SlaveAddr & ~1);
	if (-KGI_EOK == r) {
	    for (; nWrite > 0; WriteBuffer++, nWrite--)
		if (-KGI_EOK != (r = i2c_io->PutByte(d, *WriteBuffer))) 
		    break;
	    s++;
	}
    }

    if (-KGI_EOK == r && nRead > 0) {
	r = i2c_io->Address(d, d->SlaveAddr | 1);
	if (-KGI_EOK == r) {
	    for (; nRead > 0; ReadBuffer++, nRead--)
		if (-KGI_EOK != (r = i2c_io->GetByte(d, ReadBuffer, nRead == 1)))
		    break;
	    s++;
	}
    }

    if (s) i2c_io->Stop(d);

    return r;
}

/* wrapper - for compatibility and convenience */

static kgi_error_t
i2c_WriteRead(I2CDevRec * d,
		 kgi_u8_t *WriteBuffer, kgi_u_t nWrite,
		 kgi_u8_t *ReadBuffer,  kgi_u_t nRead) 
{
    kgim_i2c_io_t *i2c_io = d->pI2CBus;
    return i2c_io->WriteRead(d,WriteBuffer,nWrite,ReadBuffer,nRead);
}

/* Read a byte from one of the registers determined by its sub-address.
 */

static kgi_error_t
i2c_ReadByte(I2CDevRec * d, kgi_u8_t subaddr, kgi_u8_t *pbyte)
{
    return i2c_WriteRead(d, &subaddr, 1, pbyte, 1);
}

/* Read bytes from subsequent registers determined by the 
 * sub-address of the first register.
 */

static kgi_error_t
i2c_ReadBytes(I2CDevRec * d, kgi_u8_t subaddr, kgi_u8_t *pbyte, kgi_u_t n)
{
    return i2c_WriteRead(d, &subaddr, 1, pbyte, n);
}

/* Read a word (high byte, then low byte) from one of the registers 
 * determined by its sub-address.
 */

static kgi_error_t
i2c_ReadWord(I2CDevRec * d, kgi_u8_t subaddr, unsigned short *pword)
{
    kgi_u8_t rb[2];

    if (!i2c_WriteRead(d, &subaddr, 1, rb, 2)) return KGI_ERRNO(MONITOR,UNKNOWN);

    *pword = (rb[0] << 8) | rb[1];

    return -KGI_EOK;
}

/* Write a byte to one of the registers determined by its sub-address.
 */

static kgi_error_t
i2c_WriteByte(I2CDevRec * d, kgi_u8_t subaddr, kgi_u8_t byte)
{
    kgi_u8_t wb[2];

    wb[0] = subaddr;
    wb[1] = byte;

    return i2c_WriteRead(d, wb, 2, NULL, 0);
}

/* Write bytes to subsequent registers determined by the 
 * sub-address of the first register.
 */

static kgi_error_t
i2c_WriteBytes(I2CDevRec * d, kgi_u8_t subaddr, 
		  kgi_u8_t *WriteBuffer, kgi_u_t nWrite)
{
    kgim_i2c_io_t *i2c_io = d->pI2CBus;
    kgi_error_t r = -KGI_EOK;

    if (nWrite > 0) {
	r = i2c_io->Address(d, d->SlaveAddr & ~1);
	if (r){
	    if ((r = i2c_io->PutByte(d, subaddr)))
		for (; nWrite > 0; WriteBuffer++, nWrite--)
		    if (!(r = i2c_io->PutByte(d, *WriteBuffer))) 
			break;

	    i2c_io->Stop(d);
	}
    }

    return r;
}

/* Write a word (high byte, then low byte) to one of the registers 
 * determined by its sub-address.
 */

static kgi_error_t
i2c_WriteWord(I2CDevRec * d, kgi_u8_t subaddr, unsigned short word)
{
    kgi_u8_t wb[3];

    wb[0] = subaddr;
    wb[1] = word >> 8;
    wb[2] = word & 0xFF;

    return i2c_WriteRead(d, wb, 3, NULL, 0);
}

/* Write a vector of bytes to not adjacent registers. This vector is, 
 * 1st byte sub-address, 2nd byte value, 3rd byte sub-address asf.
 * This function is intended to initialize devices. Note this function
 * exits immediately when an error occurs, some registers may
 * remain uninitialized.
 */

static kgi_error_t
i2c_WriteVec(I2CDevRec * d, kgi_u8_t *vec, kgi_u_t nValues)
{
    kgim_i2c_io_t *i2c_io = d->pI2CBus;
    kgi_error_t r = -KGI_EOK;
    kgi_u_t s = 0;

    if (nValues > 0) {
	for (; nValues > 0; nValues--, vec += 2) {
	    if (!(r = i2c_io->Address(d, d->SlaveAddr & ~1)))
	    	break;

	    s++;

	    if (!(r = i2c_io->PutByte(d, vec[0])))
		break;

	    if (!(r = i2c_io->PutByte(d, vec[1]))) 
		break;
	}

	if (s > 0) i2c_io->Stop(d);	
    }

    return r;
}

/* Administrative functions.
 * =========================
 */

/* I2C transmissions are related to an I2CDevRec you must link to a
 * previously registered bus (see i2c_BusInit) before attempting
 * to read and write data. You may call i2c_ProbeAddress first to
 * see if the device in question is present on this bus. 
 *
 * i2c_DevInit will not allocate an I2CBusRec for you, instead you
 * may enter a pointer to a statically allocated I2CDevRec or the (modified)
 * result of i2c_CreateI2CDevRec.
 *
 * If you don't specify timeouts for the device (n <= 0), it will inherit
 * the bus-wide defaults. The function returns -KGI_EOK on success.
 */

static kgi_error_t i2c_DevInit(I2CDevRec * d) 
{
    kgim_i2c_io_t *i2c_io;

    if (d == NULL ||
	(i2c_io = d->pI2CBus) == NULL ||
        (d->SlaveAddr & 1))
	return KGI_ERRNO(MONITOR,UNKNOWN);

    d->BitTimeout = i2c_io->BitTimeout;
    d->ByteTimeout = i2c_io->ByteTimeout;
    d->AcknTimeout = i2c_io->AcknTimeout;
    d->StartTimeout = i2c_io->StartTimeout;

	KRN_DEBUG(2," I2C device registered.");

    return -KGI_EOK;
}
/* I2C masters have to register themselves using this function.
 * It will not allocate an I2CBusRec for you, instead you may enter
 * a pointer to a statically allocated I2CBusRec or the (modified)
 * result of i2c_CreateI2CBusRec. Returns -KGI_EOK on success.
 *
 * At this point there won't be any traffic on the I2C bus.
 */

static kgi_error_t i2c_BusInit(kgim_i2c_io_t *i2c_io) 
{
    /* If the high level functions are not
     * supplied, use the generic functions.
     * In this case we need the low-level
     * function.
     */
    if (i2c_io->WriteRead == NULL) 
    {
        i2c_io->WriteRead=I2CWriteRead;

        if (i2c_io->PutBits == NULL ||
	    i2c_io->GetBits == NULL) 
        {
	    if (i2c_io->PutByte == NULL ||
	        i2c_io->GetByte == NULL ||
	        i2c_io->Address == NULL ||
	        i2c_io->Stop    == NULL)
	        return KGI_ERRNO(MONITOR,UNKNOWN);
        } else { 
	    i2c_io->PutByte = I2CPutByte;
	    i2c_io->GetByte = I2CGetByte;
	    i2c_io->Address = I2CAddress;
	    i2c_io->Stop    = I2CStop;
        }
     }

    if (i2c_io->UDelay == NULL)
	i2c_io->UDelay = I2CUDelay;

    if (i2c_io->HoldTime < 2) i2c_io->HoldTime = 5;
    if (i2c_io->BitTimeout <= 0) i2c_io->BitTimeout = i2c_io->HoldTime;
    if (i2c_io->ByteTimeout <= 0) i2c_io->ByteTimeout = i2c_io->HoldTime;
    if (i2c_io->AcknTimeout <= 0) i2c_io->AcknTimeout = i2c_io->HoldTime;
    if (i2c_io->StartTimeout <= 0) i2c_io->StartTimeout = i2c_io->HoldTime;
	i2c_io->RiseFallTime = RISEFALLTIME;

    KRN_DEBUG(2, "I2C bus initialized.");

    return -KGI_EOK;
}

