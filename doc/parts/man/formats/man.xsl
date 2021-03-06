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
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:fn="http://www.w3.org/2005/02/xpath-functions">
<xsl:output method="text" indent="no" />

<xsl:include href="../../formats/man.xsl" />

<xsl:template match="/man">
<xsl:apply-templates select="manhead" />
<xsl:text>
.ensure-line-break.TP
.B Note:
Due to limitations in the character encoding used for this page, some characters in the examples have been replaced by legacy substitutions.  These issues are not present in the
.SM HTML
version of this page.</xsl:text>
<xsl:for-each select="section"><xsl:text>
.ensure-line-break.SH </xsl:text><xsl:value-of select="upper-case(title)" /><xsl:text>
.ensure-line-break</xsl:text>
<xsl:apply-templates select="top/*" />
<xsl:apply-templates select="body/*" />
<xsl:for-each select="section"><xsl:text>
.ensure-line-break.SS </xsl:text><xsl:value-of select="title" /><xsl:text>
.ensure-line-break</xsl:text>
<xsl:apply-templates select="top/*" />
<xsl:apply-templates select="body/*" />
</xsl:for-each>
</xsl:for-each>
</xsl:template>

<xsl:template match="manhead">.TH <xsl:value-of select="@prog-name" /><xsl:text> </xsl:text><xsl:value-of select="@section" /> &quot;<xsl:value-of select="@modification-date" />&quot; &quot;<xsl:value-of select="left-footer" />&quot; &quot;<xsl:apply-templates select="center-header" />&quot;</xsl:template>

<xsl:template match="synopsis-table">
<xsl:for-each select="synopsis-case"><xsl:apply-templates/><xsl:text>
</xsl:text></xsl:for-each>
</xsl:template>

<xsl:template match="synopsis-case"><xsl:apply-templates/></xsl:template>

<xsl:template match="see-also-items">
  <xsl:for-each select="see-also"><xsl:apply-templates/></xsl:for-each>
</xsl:template>
<xsl:template match="manpage[@tool,@section]"><xsl:text>.BR </xsl:text>&quot;<xsl:value-of select="@tool" />&quot; &quot;(<xsl:value-of select="@section" />)&quot;<xsl:text>
.ensure-line-break</xsl:text>
</xsl:template>

<xsl:template match="man-only">
	<xsl:apply-templates select="*"/>
</xsl:template>

<xsl:template match="expand-html-doc-index">
.ensure-line-break.br
  <xsl:value-of select="/man/manhead/html-doc-index/@href" />
</xsl:template>

<xsl:template match="env-variable-list">
<xsl:apply-templates select="./*" />
</xsl:template>
<xsl:template match="env-variable-item[@name]">
<xsl:text>

.B </xsl:text><xsl:value-of select="@name" /><xsl:text>
.ensure-line-break</xsl:text>
<xsl:apply-templates select="description/*"/>
</xsl:template>

<xsl:template match="command-line-option-list">
<xsl:apply-templates select="./*" />
</xsl:template>
<xsl:template match="command-line-item">
<xsl:text>

</xsl:text><xsl:apply-templates select="parameters" />
<xsl:apply-templates select="nosep-parameter" />
<xsl:apply-templates select="description/*"/>
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

<xsl:template match="command-line-item/parameters[@flag]"><xsl:text>
.ensure-line-break.B "</xsl:text><xsl:value-of select="@flag" /><xsl:text>"
.ensure-line-break</xsl:text><xsl:apply-templates /><xsl:text>
.ensure-line-break.br
.ensure-line-break</xsl:text></xsl:template>

<xsl:template match="command-line-item/nosep-parameter[@flag]"><xsl:text>
.ensure-line-break.BI &quot;</xsl:text><xsl:value-of select="@flag" /><xsl:text>&quot; &quot;</xsl:text><xsl:value-of select="."/><xsl:text>&quot;
.ensure-line-break.br
.ensure-line-break</xsl:text></xsl:template>

<xsl:template match="tolerance-parameter[@name]">
<xsl:text>

</xsl:text>
<xsl:apply-templates select="command-line-option-derived" />
<xsl:apply-templates select="command-line-option" />
<xsl:apply-templates select="description/*"/><xsl:text>
.ensure-line-break.br
Default value: </xsl:text><xsl:apply-templates select="default" />
</xsl:template>

<xsl:template match="tolerance-parameter/command-line-option[@flag]"><xsl:text>
.ensure-line-break.B "</xsl:text><xsl:value-of select="@flag" /><xsl:text>"
.ensure-line-break</xsl:text><xsl:apply-templates /><xsl:text>
.ensure-line-break.br
.ensure-line-break</xsl:text></xsl:template>
<xsl:template match="tolerance-parameter[@name]/command-line-option-derived"><xsl:text>
.ensure-line-break.BI "--</xsl:text><xsl:value-of select="../@name" /><xsl:text>=" "</xsl:text><xsl:value-of select="." /><xsl:text>"
.ensure-line-break.br
.ensure-line-break</xsl:text></xsl:template>


<xsl:template match="exit-code-list">
<xsl:apply-templates select="/man/external/exit-code" />
</xsl:template>

<xsl:template match="exit-code[@value]">
<xsl:text>

.B </xsl:text><xsl:value-of select="@value" /> — <xsl:apply-templates select="short" /><xsl:text>
.ensure-line-break.br
</xsl:text>
<xsl:apply-templates select="description/*"/>
</xsl:template>

<xsl:template match="prog-name[@class='other']"><xsl:value-of select="." /></xsl:template>
<xsl:template match="prog-name"><xsl:text>
.ensure-line-break.B "</xsl:text><xsl:value-of select="." /><xsl:text>"
.ensure-line-break</xsl:text></xsl:template>

<xsl:template match="synopsis-case/syntax-name"><xsl:text>
.ensure-line-break.B "</xsl:text><xsl:value-of select="." /><xsl:text>"
.ensure-line-break</xsl:text></xsl:template>
<xsl:template match="synopsis-case/bnf"><xsl:text>
.ensure-line-break </xsl:text><xsl:value-of select="." /><xsl:text>
.ensure-line-break</xsl:text></xsl:template>
<xsl:template match="p/syntax-name"><xsl:text>
.ensure-line-break.B "</xsl:text><xsl:value-of select="." /><xsl:text>"
.ensure-line-break</xsl:text></xsl:template>
<xsl:template match="synopsis-case/replacable"><xsl:text>
.ensure-line-break.I "</xsl:text><xsl:value-of select="." /><xsl:text>"
.ensure-line-break</xsl:text></xsl:template>
<xsl:template match="parameters/replacable"><xsl:text>
.ensure-line-break.I "</xsl:text><xsl:value-of select="." /><xsl:text>"
.ensure-line-break</xsl:text></xsl:template>
<xsl:template match="command-line-option/replacable"><xsl:text>
.ensure-line-break.I "</xsl:text><xsl:value-of select="." /><xsl:text>"
.ensure-line-break</xsl:text></xsl:template>

<xsl:template match="p/em"><xsl:text>
.ensure-line-break.I </xsl:text><xsl:value-of select="." /><xsl:text>
.ensure-line-break</xsl:text></xsl:template>
<xsl:template match="p/b"><xsl:text>
.ensure-line-break.B </xsl:text><xsl:value-of select="." /><xsl:text>
.ensure-line-break</xsl:text></xsl:template>

</xsl:stylesheet>
