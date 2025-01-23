PKG_VERSION ?=
ZEROTIER_VERSION ?=
GITHUB_OWNER ?=
GITHUB_REPO ?=

.if empty(PKG_VERSION)
.error PKG_VERSION is not set
.endif

.if empty(ZEROTIER_VERSION)
.error ZEROTIER_VERSION is not set
.endif

.if empty(GITHUB_OWNER)
.error GITHUB_OWNER is not set
.endif

.if empty(GITHUB_REPO)
.error GITHUB_REPO is not set
.endif

PORTNAME=	pfSense-pkg-zerotier
PORTVERSION=	${ZEROTIER_VERSION}.${PKG_VERSION}
CATEGORIES=	net
MASTER_SITES=	# empty
DISTFILES=	# empty
EXTRACT_ONLY=	# empty

MAINTAINER=	vitali.khlebko@vetal.ca
COMMENT=	pfSense package zerotier

LICENSE=	APACHE20

NO_ARCH=	yes
NO_BUILD=	yes
NO_MTREE=	yes

SUB_FILES=	pkg-install pkg-deinstall
SUB_LIST=	PORTNAME=${PORTNAME}

do-extract:
	${MKDIR} ${WRKSRC}

do-install:
	@echo "STAGEDIR is set to ${STAGEDIR}"
	@echo "PREFIX is set to ${PREFIX}"
	# Create necessary directories
	${MKDIR} ${STAGEDIR}${PREFIX}/sbin

	# Run pkg-install script
	env STAGEDIR=${STAGEDIR} PREFIX=${PREFIX} FILESDIR=${FILESDIR} DATADIR=${DATADIR} REINPLACE_CMD="${REINPLACE_CMD}" PKGVERSION=${PORTVERSION} ${SH} ${WRKDIR}/pkg-install

.include <bsd.port.mk>