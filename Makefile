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
SUB_LIST=	PORTNAME=${PORTNAME} \
		GITHUB_OWNER=${GITHUB_OWNER} \
		GITHUB_REPO=${GITHUB_REPO} \
		ZEROTIER_VERSION=${ZEROTIER_VERSION}

do-extract:
	${MKDIR} ${WRKSRC}

do-install:
	@echo "================================================"
	@echo "DEBUG: Environment variables in do-install"
	@echo "================================================"
	@echo "STAGEDIR is set to ${STAGEDIR}" | tee -a /tmp/zerotier-make.log
	@echo "PREFIX is set to ${PREFIX}" | tee -a /tmp/zerotier-make.log
	@echo "FILESDIR is set to ${FILESDIR}" | tee -a /tmp/zerotier-make.log
	@echo "DATADIR is set to ${DATADIR}" | tee -a /tmp/zerotier-make.log
	@echo "REINPLACE_CMD is set to ${REINPLACE_CMD}" | tee -a /tmp/zerotier-make.log
	@echo "PKGVERSION is set to ${PKGVERSION}" | tee -a /tmp/zerotier-make.log
	@echo "PORTVERSION is set to ${PORTVERSION}" | tee -a /tmp/zerotier-make.log
	@echo "WRKDIR is set to ${WRKDIR}" | tee -a /tmp/zerotier-make.log
	@echo "SH is set to ${SH}" | tee -a /tmp/zerotier-make.log
	@echo "================================================" | tee -a /tmp/zerotier-make.log
	# Create necessary directories
	${MKDIR} ${STAGEDIR}${PREFIX}/sbin
	${MKDIR} ${STAGEDIR}${PREFIX}/pkg
	${MKDIR} ${STAGEDIR}${PREFIX}/www
	${MKDIR} ${STAGEDIR}${DATADIR}
	${MKDIR} ${STAGEDIR}${PREFIX}/var/db/zerotier-one

	# Install files
	${INSTALL_DATA} ${FILESDIR}${PREFIX}/pkg/zerotier.xml ${STAGEDIR}${PREFIX}/pkg/
	${INSTALL_DATA} ${FILESDIR}${PREFIX}/pkg/zerotier.inc ${STAGEDIR}${PREFIX}/pkg/
	${INSTALL_DATA} ${FILESDIR}${DATADIR}/info.xml ${STAGEDIR}${DATADIR}/
	${INSTALL_DATA} ${FILESDIR}${PREFIX}/www/zerotier.php ${STAGEDIR}${PREFIX}/www/
	${INSTALL_DATA} ${FILESDIR}${PREFIX}/www/zerotier_networks.php ${STAGEDIR}${PREFIX}/www/
	${INSTALL_DATA} ${FILESDIR}${PREFIX}/www/zerotier_peers.php ${STAGEDIR}${PREFIX}/www/
	${INSTALL_DATA} ${FILESDIR}${PREFIX}/www/zerotier_controller.php ${STAGEDIR}${PREFIX}/www/
	${INSTALL_DATA} ${FILESDIR}${PREFIX}/www/zerotier_controller_network.php ${STAGEDIR}${PREFIX}/www/
	${INSTALL_DATA} ${FILESDIR}${PREFIX}/var/db/zerotier-one/local.conf ${STAGEDIR}${PREFIX}/var/db/zerotier-one/

	# Run pkg-install script
	@echo "================================================" | tee -a /tmp/zerotier-make.log
	@echo "DEBUG: Environment for pkg-install" | tee -a /tmp/zerotier-make.log
	@echo "================================================" | tee -a /tmp/zerotier-make.log
	@echo "STAGEDIR=${STAGEDIR}" | tee -a /tmp/zerotier-make.log
	@echo "PREFIX=${PREFIX}" | tee -a /tmp/zerotier-make.log
	@echo "FILESDIR=${FILESDIR}" | tee -a /tmp/zerotier-make.log
	@echo "DATADIR=${DATADIR}" | tee -a /tmp/zerotier-make.log
	@echo "REINPLACE_CMD=${REINPLACE_CMD}" | tee -a /tmp/zerotier-make.log
	@echo "PKGVERSION=${PORTVERSION}" | tee -a /tmp/zerotier-make.log
	@echo "GITHUB_OWNER=${GITHUB_OWNER}" | tee -a /tmp/zerotier-make.log
	@echo "GITHUB_REPO=${GITHUB_REPO}" | tee -a /tmp/zerotier-make.log
	@echo "ZEROTIER_VERSION=${ZEROTIER_VERSION}" | tee -a /tmp/zerotier-make.log
	@echo "================================================" | tee -a /tmp/zerotier-make.log
	env STAGEDIR=${STAGEDIR} \
		PREFIX=${PREFIX} \
		FILESDIR=${FILESDIR} \
		DATADIR=${DATADIR} \
		REINPLACE_CMD="${REINPLACE_CMD}" \
		PKGVERSION=${PORTVERSION} \
		GITHUB_OWNER=${GITHUB_OWNER} \
		GITHUB_REPO=${GITHUB_REPO} \
		ZEROTIER_VERSION=${ZEROTIER_VERSION} \
		${SH} ${WRKDIR}/pkg-install

.include <bsd.port.mk>