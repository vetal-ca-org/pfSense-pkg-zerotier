# Read versions from VERSION file
include VERSION

PORTNAME=	pfSense-pkg-zerotier
PORTVERSION=	${ZEROTIER_VERSION}.${PKG_VERSION}
CATEGORIES=	net
MASTER_SITES=	# empty
DISTFILES=	# empty
EXTRACT_ONLY=	# empty

MAINTAINER=	vitali.khlebko@vetal.ca
COMMENT=	pfSense package zerotier

LICENSE=	APACHE20

RUN_DEPENDS=	${LOCALBASE}/sbin/zerotier-one:net/zerotier

NO_BUILD=	yes
NO_MTREE=	yes

FILESDIR=	${.CURDIR}/files
SUB_FILES=	pkg-install pkg-deinstall
SUB_LIST=	PREFIX=${PREFIX} STAGEDIR=${STAGEDIR} DATADIR=${DATADIR} PKGVERSION=${PORTVERSION} ZEROTIER_VERSION=${ZEROTIER_VERSION}

# Set DATADIR explicitly
DATADIR=	${PREFIX}/share/${PORTNAME}

# Set REINPLACE_CMD
REINPLACE_CMD=	sed -i ''

do-extract:
	${MKDIR} ${WRKSRC}

do-install:
	# Create necessary directories
	${MKDIR} ${STAGEDIR}${PREFIX}/sbin

	# Run pkg-install script
	env STAGEDIR=${STAGEDIR} PREFIX=${PREFIX} FILESDIR=${FILESDIR} DATADIR=${DATADIR} REINPLACE_CMD="${REINPLACE_CMD}" PKGVERSION=${PORTVERSION} ${SH} ${WRKDIR}/pkg-install

	# Install post-install and post-deinstall scripts directly into STAGEDIR
	${INSTALL_SCRIPT} ${FILESDIR}/post-install.sh ${STAGEDIR}${PREFIX}/sbin/${PORTNAME}-post-install
	${INSTALL_SCRIPT} ${FILESDIR}/post-deinstall.sh ${STAGEDIR}${PREFIX}/sbin/${PORTNAME}-post-deinstall

	# Create +POST_INSTALL file
	echo "#!/bin/sh" > ${STAGEDIR}/+POST_INSTALL
	echo "${PREFIX}/sbin/${PORTNAME}-post-install" >> ${STAGEDIR}/+POST_INSTALL
	chmod +x ${STAGEDIR}/+POST_INSTALL

	# Create +POST_DEINSTALL file
	echo "#!/bin/sh" > ${STAGEDIR}/+POST_DEINSTALL
	echo "${PREFIX}/sbin/${PORTNAME}-post-deinstall" >> ${STAGEDIR}/+POST_DEINSTALL
	chmod +x ${STAGEDIR}/+POST_DEINSTALL

do-deinstall:
	${SH} ${WRKDIR}/pkg-deinstall

.include <bsd.port.mk>