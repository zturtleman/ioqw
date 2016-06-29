/*
=======================================================================================================================================
Copyright (C) 2016 James Canete.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Spearmint Source Code.
If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms. You should have received a copy of these additional
terms immediately following the terms and conditions of the GNU General Public License. If not, please request a copy in writing from
id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
=======================================================================================================================================
*/

#ifndef __TR_DSA_H__
#define __TR_DSA_H__

#include "../renderercommon/qgl.h"

void GL_BindNullTextures(void);
int GL_BindMultiTexture(GLenum texunit, GLenum target, GLuint texture);

GLvoid APIENTRY GLDSA_BindMultiTexture(GLenum texunit, GLenum target, GLuint texture);
GLvoid APIENTRY GLDSA_TextureParameterf(GLuint texture, GLenum target, GLenum pname, GLfloat param);
GLvoid APIENTRY GLDSA_TextureParameteri(GLuint texture, GLenum target, GLenum pname, GLint param);
GLvoid APIENTRY GLDSA_TextureImage2D(GLuint texture, GLenum target, GLint level, GLint internalformat,
	GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLvoid APIENTRY GLDSA_TextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GLvoid APIENTRY GLDSA_CopyTextureImage2D(GLuint texture, GLenum target, GLint level, GLenum internalformat,
	GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GLvoid APIENTRY GLDSA_CompressedTextureImage2D(GLuint texture, GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
GLvoid APIENTRY GLDSA_CompressedTextureSubImage2D(GLuint texture, GLenum target, GLint level,
	GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format,
	GLsizei imageSize, const GLvoid *data);

GLvoid APIENTRY GLDSA_GenerateTextureMipmap(GLuint texture, GLenum target);

void GL_BindNullProgram(void);
int GL_UseProgramObject(GLuint program);

GLvoid APIENTRY GLDSA_ProgramUniform1i(GLuint program, GLint location, GLint v0);
GLvoid APIENTRY GLDSA_ProgramUniform1f(GLuint program, GLint location, GLfloat v0);
GLvoid APIENTRY GLDSA_ProgramUniform2f(GLuint program, GLint location,
	GLfloat v0, GLfloat v1);
GLvoid APIENTRY GLDSA_ProgramUniform3f(GLuint program, GLint location,
	GLfloat v0, GLfloat v1, GLfloat v2);
GLvoid APIENTRY GLDSA_ProgramUniform4f(GLuint program, GLint location,
	GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLvoid APIENTRY GLDSA_ProgramUniform1fv(GLuint program, GLint location,
	GLsizei count, const GLfloat *value);
GLvoid APIENTRY GLDSA_ProgramUniformMatrix4fv(GLuint program, GLint location,
	GLsizei count, GLboolean transpose,
	const GLfloat *value);

void GL_BindNullFramebuffers(void);
void GL_BindFramebuffer(GLenum target, GLuint framebuffer);
void GL_BindRenderbuffer(GLuint renderbuffer);

GLvoid APIENTRY GLDSA_NamedRenderbufferStorage(GLuint renderbuffer,
	GLenum internalformat, GLsizei width, GLsizei height);

GLvoid APIENTRY GLDSA_NamedRenderbufferStorageMultisample(GLuint renderbuffer,
	GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

GLenum APIENTRY GLDSA_CheckNamedFramebufferStatus(GLuint framebuffer, GLenum target);
GLvoid APIENTRY GLDSA_NamedFramebufferTexture2D(GLuint framebuffer,
	GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GLvoid APIENTRY GLDSA_NamedFramebufferRenderbuffer(GLuint framebuffer,
	GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);


#endif
