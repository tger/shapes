<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" indent="no" />

<xsl:include href="../../formats/html.xsl" />
<xsl:include href="../../formats/examplecode-html.xsl" />
<xsl:include href="../../formats/plain-book-html.xsl" />
<xsl:include href="../../formats/language-elements-html.xsl" />

<xsl:template match="alphabetical-index">
	<p class="center"><b>Alphabetical index</b></p>
	<p class="center">
		<xsl:for-each select="/book//system-binding[@identifier]">
			<xsl:sort select="@identifier" />
			<xsl:call-template name="name-to-linked-binding">
				<xsl:with-param name="extension-href"><xsl:apply-templates select="/book/meta-selflink" /></xsl:with-param>
				<xsl:with-param name="name"><xsl:value-of select="@identifier" /></xsl:with-param>
			</xsl:call-template>
			  
		</xsl:for-each>
	</p>
	<p class="center">
		<xsl:for-each select="/book//dynamic-variable[@identifier]">
			<xsl:sort select="@identifier" />
			<xsl:call-template name="name-to-linked-dynvar">
				<xsl:with-param name="extension-href"><xsl:apply-templates select="/book/meta-selflink" /></xsl:with-param>
				<xsl:with-param name="name"><xsl:value-of select="@identifier" /></xsl:with-param>
			</xsl:call-template>
			  
		</xsl:for-each>
	</p>
	<hr class="thin"/>
	<xsl:if test="/book/needs/a[@extension]">
		<p>
			<b>Dependencies:</b>
			<xsl:for-each select="/book/needs/a[@extension]">
				  
				<xsl:apply-templates select="."/>
			</xsl:for-each>
		</p>
		<hr class="thin"/>
	</xsl:if>
</xsl:template>

<xsl:template match="index-of-books">
	<ul>
		<xsl:apply-templates select="/book/external/book | /book/external/man" />
	</ul>
</xsl:template>

<xsl:template match="external/book">
	<li>
		<xsl:element name="a">
			<xsl:attribute name="href"><xsl:apply-templates select="meta-selflink" /></xsl:attribute>
			<b><xsl:apply-templates select="title" /></b>
		</xsl:element>:
		<xsl:apply-templates select="description" />
	</li>
</xsl:template>

</xsl:stylesheet>
