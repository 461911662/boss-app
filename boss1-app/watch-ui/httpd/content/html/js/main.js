/**
 * @file main.js
 * @brief Main JavaScript for captive portal
 * @author BOSS1
 * @version 1.0
 * @date 2024
 */

(function() {
    'use strict';

    window.WatchUI = window.WatchUI || {};

    WatchUI.api = {
        baseUrl: '/cgi-bin/cgi',

        async request(endpoint, data) {
            const url = this.baseUrl + endpoint;
            const response = await fetch(url, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data || {}),
            });
            return response.json();
        },

        async login(username, password) {
            return this.request('/login', { username, password });
        },

        async getStatus() {
            return this.request('/status', {});
        },

        async logout() {
            return this.request('/logout', {});
        }
    };

    WatchUI.utils = {
        showError(message) {
            const errorEl = document.getElementById('error-message');
            if (errorEl) {
                errorEl.textContent = message;
                errorEl.style.display = 'block';
            }
        },

        hideError() {
            const errorEl = document.getElementById('error-message');
            if (errorEl) {
                errorEl.style.display = 'none';
            }
        },

        showLoading() {
            const btn = document.querySelector('button[type="submit"]');
            if (btn) {
                btn.disabled = true;
                btn.dataset.originalText = btn.textContent;
                btn.textContent = 'Loading...';
            }
        },

        hideLoading() {
            const btn = document.querySelector('button[type="submit"]');
            if (btn && btn.dataset.originalText) {
                btn.disabled = false;
                btn.textContent = btn.dataset.originalText;
            }
        }
    };

    document.addEventListener('DOMContentLoaded', function() {
        // Flash animation - hide after animation completes
        window.addEventListener('load', function() {
            setTimeout(function() {
                const flash = document.getElementById('flash');
                if (flash) {
                    flash.classList.add('hide');
                }
            }, 2500);
        });

        // Theme toggle
        (function() {
            const toggle = document.getElementById('theme-toggle');
            const html = document.documentElement;
            
            if (!toggle) return;
            
            const savedTheme = localStorage.getItem('theme') || 'light';
            html.setAttribute('data-theme', savedTheme);
            
            toggle.addEventListener('click', function() {
                const current = html.getAttribute('data-theme');
                const next = current === 'light' ? 'dark' : 'light';
                html.setAttribute('data-theme', next);
                localStorage.setItem('theme', next);
            });
        })();

        // Menu toggle
        (function() {
            const menuToggle = document.getElementById('menu-toggle');
            const menuPanel = document.getElementById('menu-panel');
            const menuClose = document.getElementById('menu-close');
            
            if (!menuToggle || !menuPanel) return;
            
            menuToggle.addEventListener('click', function() {
                menuPanel.classList.add('active');
            });
            
            if (menuClose) {
                menuClose.addEventListener('click', function() {
                    menuPanel.classList.remove('active');
                });
            }
            
            menuPanel.addEventListener('click', function(e) {
                if (e.target === menuPanel) {
                    menuPanel.classList.remove('active');
                }
            });
        })();

        // Accordion - Help
        (function() {
            const toggle = document.getElementById('help-toggle');
            const content = document.getElementById('help-content');
            
            if (toggle && content) {
                toggle.addEventListener('click', function() {
                    content.classList.toggle('expanded');
                    toggle.classList.toggle('expanded');
                });
            }
        })();

        // Accordion - About Me
        (function() {
            const toggle = document.getElementById('about-toggle');
            const content = document.getElementById('about-content');
            
            if (toggle && content) {
                toggle.addEventListener('click', function() {
                    content.classList.toggle('expanded');
                    toggle.classList.toggle('expanded');
                });
            }
        })();

        // Auth Form Handler
        (function() {
            const form = document.getElementById('auth-form');
            const errorMsg = document.getElementById('error-msg');
            const successMsg = document.getElementById('success-msg');
            const submitBtn = document.getElementById('submit-btn');
            const usernameInput = document.getElementById('username');
            const passwordInput = document.getElementById('password');

            if (!form) return;

            form.addEventListener('submit', async function(e) {
                e.preventDefault();
                
                const username = usernameInput.value.trim();
                const password = passwordInput.value;

                if (!username || !password) {
                    showError('请输入账号和密码');
                    return;
                }

                submitBtn.disabled = true;
                submitBtn.textContent = '处理中...';
                hideMessages();

                try {
                    const loginResp = await fetch('/cgi-bin/cgi/login', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ username, password })
                    });
                    
                    const loginData = await loginResp.json();

                    if (loginData.success) {
                        showSuccess('登录成功！欢迎回来，' + username);
                        return;
                    }

                    if (loginData.error === 'user_not_found' || loginData.error === 'invalid_password') {
                        const registerResp = await fetch('/cgi-bin/cgi/register', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({ username, password })
                        });
                        
                        const registerData = await registerResp.json();
                        
                        if (registerData.success) {
                            showSuccess('注册成功！欢迎加入 BOSS1');
                        } else {
                            showError(registerData.error || '操作失败');
                        }
                    } else {
                        showError(loginData.error || '操作失败');
                    }
                } catch (err) {
                    console.error('Auth error:', err);
                    showSuccess('操作成功');
                }

                submitBtn.disabled = false;
                submitBtn.textContent = '进入';
            });

            function showError(msg) {
                errorMsg.textContent = msg;
                errorMsg.style.display = 'block';
                successMsg.style.display = 'none';
            }

            function showSuccess(msg) {
                successMsg.textContent = msg;
                successMsg.style.display = 'block';
                errorMsg.style.display = 'none';
            }

            function hideMessages() {
                errorMsg.style.display = 'none';
                successMsg.style.display = 'none';
            }
        })();

        // Menu panel handling
        const menuPanel = document.getElementById('menu-panel');
        const menuToggle = document.getElementById('menu-toggle');
        const menuClose = document.getElementById('menu-close');

        function openMenu() {
            if (menuPanel) {
                menuPanel.classList.add('active');
                document.body.style.overflow = 'hidden';
            }
        }

        function closeMenu() {
            if (menuPanel) {
                menuPanel.classList.remove('active');
                document.body.style.overflow = '';
            }
        }

        if (menuToggle) {
            menuToggle.addEventListener('click', openMenu);
        }

        if (menuClose) {
            menuClose.addEventListener('click', closeMenu);
        }

        // Handle TOC item clicks
        const tocItems = document.querySelectorAll('.toc-item');
        tocItems.forEach(function(item) {
            item.addEventListener('click', function(e) {
                const href = this.getAttribute('href');
                if (href && href.startsWith('#')) {
                    e.preventDefault();
                    const targetId = href.substring(1);
                    const targetEl = document.getElementById(targetId);
                    if (targetEl) {
                        targetEl.scrollIntoView({ behavior: 'smooth' });
                        setTimeout(function() {
                            expandAccordion(targetEl);
                        }, 300);
                    }
                }
            });
        });

        // Handle menu item clicks
        const menuItems = document.querySelectorAll('.menu-item');
        menuItems.forEach(function(item) {
            item.addEventListener('click', function(e) {
                const href = this.getAttribute('href');
                if (href && href.startsWith('#')) {
                    e.preventDefault();
                    const targetId = href.substring(1);
                    const targetEl = document.getElementById(targetId);
                    if (targetEl) {
                        targetEl.scrollIntoView({ behavior: 'smooth' });
                        setTimeout(function() {
                            expandAccordion(targetEl);
                        }, 300);
                    }
                    closeMenu();
                }
            });
        });

        // Expand accordion function
        function expandAccordion(element) {
            if (element.classList.contains('accordion-card')) {
                const content = element.querySelector('.accordion-content');
                const header = element.querySelector('.accordion-header');
                if (content && header && !content.classList.contains('expanded')) {
                    content.classList.add('expanded');
                    header.classList.add('expanded');
                }
            }
        }
    });

})();
