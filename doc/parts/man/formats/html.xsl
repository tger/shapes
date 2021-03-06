<?xml version="1.0" encoding="UTF-8"?>

<!-- This file is part of Shapes.                                           -->
<!--                                                                        -->
<!-- Shapes is free software: you can redistribute it and/or modify         -->
<!-- it under the terms of the GNU General Public License as published by   -->
<!-- the Free Software Foundation, either version 3 of the License, or      -->
<!-- any later version.                                                     -->
<!--                                                                        -->
<!-- Shapes is distributed in the hope that it will be useful,              -->
<!-- but WITHOUT ANY WARRANTY; without even the implied warranty of         -->
<!-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          -->
<!-- GNU General Public License for more details.                           -->
<!--                                                                        -->
<!-- You should have received a copy of the GNU General Public License      -->
<!-- along with Shapes.  If not, see <http://www.gnu.org/licenses/>.        -->
<!--                                                                        -->
<!-- Copyright 2008 Henrik Tidefelt                                         -->

<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" indent="no" />

<xsl:include href="../../formats/html.xsl" />

<xsl:template match="/man">
  <html>
    <head>
      <title><xsl:apply-templates select="manhead/center-header" /></title>
			<xsl:element name="link">
				<xsl:attribute name="rel">stylesheet</xsl:attribute>
				<xsl:attribute name="href"><xsl:value-of select="/book/base/@href" />shapes.css</xsl:attribute>
			</xsl:element>
    </head>
    <body>
			<xsl:call-template name="head-navigation" />
			<xsl:apply-templates select="manhead" />
			<hr class="thick"/>
			<xsl:for-each select="section">
				<h3><xsl:value-of select="title" /></h3>
				<xsl:apply-templates select="top/*" />
				<xsl:apply-templates select="body/*" />
				<xsl:for-each select="section">
					<h4><xsl:value-of select="title" /></h4>
					<xsl:apply-templates select="top/*" />
					<xsl:apply-templates select="body/*" />
				</xsl:for-each>
			</xsl:for-each>
			<xsl:call-template name="SF-placeholder" />
		</body>
  </html>
</xsl:template>

<xsl:template match="manhead">
	<h2><xsl:apply-templates select="center-header" /></h2>
	<p>This page corresponds to the man page <b><xsl:value-of select="@prog-name" /></b>(<xsl:value-of select="@section" />), dated <xsl:value-of select="@modification-date" />.</p>
	<p><em><b>Note:</b> The examples here may be presented using characters that may not be available in the man page format.  Hence, the examples may be presented differently in the man page.</em></p>
</xsl:template>

<xsl:template match="synopsis-table">
<xsl:for-each select="synopsis-case">
	<p><xsl:apply-templates/></p>
</xsl:for-each>
</xsl:template>

<xsl:template match="see-also-items">
  <xsl:for-each select="see-also"><xsl:apply-templates/><xsl:text> </xsl:text></xsl:for-each>
</xsl:template>
<xsl:template match="manpage[@tool,@section]">
	<xsl:element name="a"><xsl:attribute name="href">http://www.google.com/search?btnI=I%27m+Feeling+Lucky&amp;q=%22<xsl:value-of select="@tool" />(<xsl:value-of select="@section" />)%22</xsl:attribute><b><xsl:value-of select="@tool" /></b>(<xsl:value-of select="@section" />)</xsl:element>
</xsl:template>

<xsl:template match="man-only">
<!-- Ignore in HTML output -->
</xsl:template>

<xsl:template match="env-variable-list">
<ul><xsl:apply-templates /></ul>
</xsl:template>
<xsl:template match="env-variable-item[@name]">
<li>
	<xsl:element name="a">
		<xsl:attribute name="name"><xsl:call-template name="env-var-to-anchor-name"><xsl:with-param name="id"><xsl:value-of select="@name" /></xsl:with-param></xsl:call-template></xsl:attribute>
		<b><xsl:value-of select="@name" /></b>
	</xsl:element>
	<xsl:apply-templates select="description/*"/>
</li>
</xsl:template>

<xsl:template match="command-line-option-list">
<ul><xsl:apply-templates /></ul>
</xsl:template>
<xsl:template match="command-line-item">
<li>
<xsl:if test="./@id">
	<xsl:element name="a">
		<xsl:attribute name="name"><xsl:call-template name="id-to-anchor-name"><xsl:with-param name="id"><xsl:value-of select="@id" /></xsl:with-param></xsl:call-template></xsl:attribute>
	</xsl:element>
</xsl:if>
<xsl:apply-templates select="parameters" />
<xsl:apply-templates select="nosep-parameter" />
<xsl:apply-templates select="description/*"/>
</li>
</xsl:template>
<xsl:template match="command-line-tolparam[@name]">
  <xsl:variable name="tolname">
    <xsl:value-of select="@name" />
  </xsl:variable>
	<xsl:apply-templates select="/man/external/tolerance-parameter[@name=$tolname]" />
</xsl:template>
<xsl:template match="command-line-tolparam-remaining">
	<xsl:for-each select="/man/external/tolerance-parameter">
		<xsl:variable name="tolname">
			<xsl:value-of select="@name" />
		</xsl:variable>
		<xsl:if test="not(//command-line-tolparam[@name=$tolname])">
			<xsl:apply-templates select="." />
		</xsl:if>
	</xsl:for-each>
</xsl:template>

<xsl:template match="command-line-item/parameters[@flag]">
	<b><xsl:value-of select="@flag" /></b> <xsl:apply-templates /><br />
</xsl:template>

<xsl:template match="command-line-item/nosep-parameter[@flag]">
	<b><xsl:value-of select="@flag" /></b><program-param class="replacable"><xsl:value-of select="."/></program-param><br />
</xsl:template>

<xsl:template match="tolerance-parameter[@name]">
	<li>
		<xsl:apply-templates select="command-line-option-derived" />
		<xsl:apply-templates select="command-line-option" />
		<xsl:apply-templates select="description/*"/>
		<p>Default value: <xsl:apply-templates select="default" /></p>
	</li>
</xsl:template>

<xsl:template match="tolerance-parameter/command-line-option[@flag]">
	<b><xsl:value-of select="@flag" /></b> <xsl:apply-templates /><br />
</xsl:template>
<xsl:template match="tolerance-parameter[@name]/command-line-option-derived">
	<b>--<xsl:value-of select="../@name" />=</b><program-param class="replacable"><xsl:value-of select="." /></program-param><br />
</xsl:template>

<xsl:template match="exit-code-list">
	<ul>
		<xsl:apply-templates select="/man/external/exit-code" />
	</ul>
</xsl:template>


<xsl:template match="exit-code[@value]">
	<li>
		<b><xsl:apply-templates select="@value" /> — <xsl:apply-templates select="short" /></b><br />
		<xsl:apply-templates select="description/*" />
	</li>
</xsl:template>

<xsl:template match="prog-name[@class='other']"><xsl:value-of select="." /></xsl:template>
<xsl:template match="prog-name"><b><xsl:value-of select="." /></b></xsl:template>

<xsl:template match="synopsis-case/replacable"><program-param class="replacable"><xsl:value-of select="." /></program-param></xsl:template>
<xsl:template match="parameters/replacable"><program-param class="replacable"><xsl:value-of select="." /></program-param></xsl:template>
<xsl:template match="command-line-option/replacable"><program-param class="replacable"><xsl:value-of select="." /></program-param></xsl:template>

</xsl:stylesheet>
